/** @file concurrent_hashtable_binary_heap.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_CONCURRENT_HASHTABLE_BINARY_HEAP_H
#define NUMDB_CONCURRENT_HASHTABLE_BINARY_HEAP_H

#include <cassert>
#include <cmath>
#include <mutex>
#include <vector>
#include <atomic>
#include <ostream>
#include <memory>
#include <experimental/optional>

#include <murmurhash2/all.h>

#include "numdb/utils.h"
#include "numdb/priority.h"

namespace numdb {
	namespace containers {
		template <typename KeyT, typename ValueT,
				typename PriorityT, typename HasherT,
				bool UseBackoff, bool UseShortIndex>
		class ConcurrentHashtable {
		  public:
			using key_t = KeyT;
			using value_t = ValueT;
			using hasher_t = HasherT;
			using priority_t = PriorityT;
			using optional_value_t = std::experimental::optional<value_t>;
			using idx_t = std::conditional_t<UseShortIndex, uint32_t, size_t>;
			using lock_guard_t = std::unique_lock<std::mutex>;
			using backoff_t = utility::ExpBackoff;

			//private: //FIXME
			struct TableNode {
				void extract(TableNode** this_node_ref) {
					assert(this_node_ref != nullptr);
					assert(this == *this_node_ref);
					*this_node_ref = next_;
				}

				void insertAfter(TableNode** prev_node_ref) {
					assert(prev_node_ref != nullptr);
					next_ = *prev_node_ref;
					*prev_node_ref = this;
				}

				TableNode* next_;
				std::atomic<idx_t> heap_node_;
				key_t key_;
				value_t value_;
			};

			struct HeapNode {
				TableNode* table_node_ = nullptr;
				priority_t priority_;
				bool up_ = false; //TODO Embed into priority or pointer

				bool operator <(const HeapNode& other) const {
					return priority_ < other.priority_;
				}
			};

			struct LockedHeapNode {
				idx_t idx;
				lock_guard_t lg;
			};

		  public:
			static constexpr size_t maxElemCountForCapacity(
					size_t capacity, double load_factor) {
				return static_cast<size_t>(
						capacity / (sizeof(TableNode) + sizeof(TableNode*) / load_factor));
			}

			static constexpr size_t elementSize(double load_factor) {
				return sizeof(TableNode) + sizeof(HeapNode) + sizeof(std::mutex) +
					   (size_t) std::ceil((sizeof(TableNode*) + sizeof(std::mutex)) / load_factor);
			}

			static constexpr bool isThreadsafe() {
				return true;
			}

			ConcurrentHashtable(size_t available_memory, double load_factor = 2) :
					ConcurrentHashtable(
							maxElemCountForCapacity(available_memory, load_factor),
							load_factor, 0) {}

		  private:
			ConcurrentHashtable(size_t element_count, double load_factor, char dummy) :
					max_count_(element_count), load_factor_(load_factor), count_(0),
					buckets_(static_cast<size_t>(element_count / load_factor), nullptr),
					bucket_locks_(buckets_.size()), table_nodes_(element_count),
					heap_(element_count), heap_locks_(element_count) {
				;
				if (buckets_.size() == 0)
					throw std::invalid_argument("Can't construct fixed hashtable: insufficient memory");

				for (size_t i = 0; i < table_nodes_.size() - 1; i++)
					table_nodes_[i].next_ = &table_nodes_[i + 1];
				disposed_nodes_head_ = &table_nodes_[0];
			}

		  public:
			ConcurrentHashtable(const ConcurrentHashtable&) = delete;
			ConcurrentHashtable& operator =(const ConcurrentHashtable&) = delete;

			~ConcurrentHashtable() = default;

			size_t capacity() const {
				return max_count_;
			}

			size_t size() const {
				return count_;
			}

			size_t elementSize() const {
				return elementSize(load_factor_);
			}

			void dump(std::ostream& out) const {
				for (size_t i = 0; i < buckets_.size(); i++) {
					out << '(' << i << ')';
					lock_guard_t lg((buckets_[i].lock_));
					TableNode* node = buckets_[i].ptr_;

					while (node) {
						out << "->" << node->key_ << '[' << node->value_ << ']';
						node = node->next_;
					}
					out << std::endl;
				}
			}

			optional_value_t find(const key_t& key) {
				size_t bucket_nr = getBucket(key);
				TableNode** node_ref = &buckets_[bucket_nr];
				lock_guard_t lg((bucket_locks_[bucket_nr]));

				while (*node_ref) {
					if (key < (*node_ref)->key_)
						break;
					if (key == (*node_ref)->key_) {
						TableNode* found_node = *node_ref;
						heapIncreasePriority(found_node);
						return {found_node->value_};
					}
					node_ref = &((*node_ref)->next_);
				}

				return {};
			}

			bool insert(key_t key, value_t value, size_t priority) {
				auto deleter = [&](TableNode* node) {
					disposeNode(node);
				};
				std::unique_ptr<TableNode, decltype(deleter)>
						empty_node(acquireFreeNode(),
								   deleter); //Returns the node back into the pool in case of failure/exception

				size_t bucket_nr = getBucket(key);
				TableNode** node_ref = &buckets_[bucket_nr];
				lock_guard_t lg((bucket_locks_[bucket_nr]));
				while (*node_ref) {
					if (key == (*node_ref)->key_) {
						heapIncreasePriority(*node_ref);
						return false;
					}
					if (key < (*node_ref)->key_)
						break;
					node_ref = &((*node_ref)->next_);
				}

				empty_node->key_ = std::move(key);
				empty_node->value_ = std::move(value);
				empty_node->insertAfter(node_ref);
				heapInsert(empty_node.get(), std::move(lg));
				empty_node.release();
				return true;
			}

		  private:
			static constexpr idx_t null = static_cast<idx_t>(-1);

			size_t getBucket(const key_t& key) const {
				size_t bucket = hasher_t()(key) % buckets_.size();
				return bucket;
			}

			TableNode* extractLuNode() {
				TableNode* candidate = heapExtractMin();
				if (!candidate)
					return nullptr;

				size_t bucket_nr = getBucket(candidate->key_);
				TableNode** node_ref = &buckets_[bucket_nr];
				lock_guard_t lg((bucket_locks_[bucket_nr]));

				while (*node_ref) {
					if (candidate->key_ < (*node_ref)->key_)
						break;
					if (candidate->key_ == (*node_ref)->key_) {
						TableNode* found_node = *node_ref;
						assert(found_node == candidate);
						found_node->extract(node_ref);
						return found_node;
					}
					node_ref = &(*node_ref)->next_;
				}
				assert(false);
				return nullptr;
			}

			TableNode* acquireFreeNode() {
				TableNode* node = nullptr;
				while (1) {
					if (node = allocNode())
						break;
					if (node = extractLuNode())
						break;
				}
				return node;
			}

			TableNode* allocNode() {
				if (!disposed_nodes_head_)
					return nullptr;

				lock_guard_t lg(disposed_nodes_lock_);
				TableNode* tnode = disposed_nodes_head_;
				if (tnode)
					disposed_nodes_head_ = tnode->next_;
				return tnode;
			}

			void disposeNode(TableNode* tnode) {
				assert(tnode);
				lock_guard_t lg(disposed_nodes_lock_);
				tnode->next_ = disposed_nodes_head_;
				disposed_nodes_head_ = tnode;
			}

			/**
			 * @attention Assumes a and b nodes are locked
			 */
			void swapHeapNodes(idx_t a, idx_t b) {
				std::swap(heap_[a], heap_[b]);
				heap_[a].table_node_->heap_node_ = a;
				heap_[b].table_node_->heap_node_ = b;
			}

			TableNode* heapExtractMin() {
				lock_guard_t lg1((heap_locks_[0]));
				if (count_ == 0)
					return nullptr;

				TableNode* evicted_node = heap_[0].table_node_;

				if (count_ == 1) {
					count_ = 0;

					return evicted_node;
				}

				{
					lock_guard_t lg2((heap_locks_[count_ - 1]));
					swapHeapNodes(0, count_ - 1);
					count_--;
				}
				if (count_ > 1) {
					topDownHeapify(0, std::move(lg1));
				}
				return evicted_node;
			}

			void heapInsert(TableNode* tnode, lock_guard_t bucket_lock) {
				assert(tnode);
				lock_guard_t lg1((heap_locks_[0]));
				assert(count_ < max_count_);
				tnode->heap_node_ = count_.load();
				if (count_ == 0) {
					heap_[count_].table_node_ = tnode;
					heap_[count_].priority_ = {};
					heap_[count_].up_ = false;
					count_ = 1;
					return;
				}

				lock_guard_t lg2((heap_locks_[count_]));
				heap_[count_].table_node_ = tnode;
				heap_[count_].priority_ = {};
				heap_[count_].up_ = true;
				count_++;
				lg1.unlock();
				bottomUpHeapify(tnode, std::move(lg2));
			}

			void heapIncreasePriority(TableNode* tnode) {
				assert(tnode);
				while (1) {
					auto lg = heapLockNode(tnode);
					assert(lg.owns_lock());
					idx_t hnode = tnode->heap_node_;
					if (heap_[hnode].up_) //TODO Add backoff?
						continue;
					heap_[hnode].priority_.access();
					topDownHeapify(hnode, std::move(lg));
					return;
				}
			}

			lock_guard_t heapLockNode(TableNode* tnode) {
				while (true) {
					idx_t hnode = tnode->heap_node_;
					//hnode == -1?
					lock_guard_t lg(heap_locks_[hnode], std::try_to_lock);
					if (lg.owns_lock()) {
						if (heap_[hnode].table_node_ == tnode) {
							assert(hnode == tnode->heap_node_);
							return std::move(lg);
						}
					}
				}
			}

			idx_t topDownHeapify(idx_t parent, lock_guard_t parent_lg) {
				assert(parent_lg.owns_lock());
				while (1) {

					auto left = heapAcquireLeftLock(parent);
					if (left.idx == null)
						return parent;

					auto right = heapAcquireRightLock(parent);

					idx_t min = 0;

					if (right.idx != null && heap_[right.idx] < heap_[left.idx]) {
						left.lg.unlock();
						min = right.idx;
					} else {
						right.lg = lock_guard_t(); //Optionally release right lock
						min = left.idx;
					}

					if (heap_[min] < heap_[parent]) {
						swapHeapNodes(parent, min);
						parent_lg = std::move(min == left.idx ? left.lg : right.lg);
						parent = min;
					} else
						return parent;
				}
			}

			idx_t bottomUpHeapify(TableNode* tnode, lock_guard_t node_lg) {
				assert(node_lg.owns_lock());
				idx_t node = tnode->heap_node_;
				backoff_t backoff;
				while (1) {
					auto parent = heapParent(node);
					if (parent == null)
						break;
					lock_guard_t parent_lg(heap_locks_[parent], std::try_to_lock);
					if (parent_lg.owns_lock()) {
						if (!heap_[parent].up_) {
							if (heap_[node].priority_ < heap_[parent].priority_)
								swapHeapNodes(node, parent);
							else
								break;
						} else {
							parent_lg.unlock();
							if (UseBackoff)
								backoff();
						}
					}
					node_lg.unlock();
					node_lg = parent_lg.owns_lock() ? std::move(parent_lg) : heapLockNode(tnode);
					node = tnode->heap_node_;
				}

				heap_[node].up_ = false;
				return node;
			}

			idx_t heapParent(idx_t i) {
				return i == 0 ? null : (i - 1) / 2;
			}

			idx_t heapLeft(idx_t i) {
				return i * 2 + 1 < max_count_ ? i * 2 + 1 : null;
			}

			idx_t heapRight(idx_t i) {
				return i * 2 + 2 < max_count_ ? i * 2 + 2 : null;
			}

			LockedHeapNode heapAcquireLeftLock(idx_t i) {
				idx_t child = heapLeft(i);
				if (child == null)
					return {null, {}};
				lock_guard_t lg((heap_locks_[child]));
				//Although count_ can be changed concurrently,
				// its value can't pass 'child' idx without locking 'child' mutex
				if (child < count_)
					return {child, std::move(lg)};
				else
					return {null, {}};
			}

			LockedHeapNode heapAcquireRightLock(idx_t i) {
				idx_t child = heapRight(i);
				if (child == null)
					return {null, {}};
				lock_guard_t lg((heap_locks_[child]));
				//Although count_ can be changed concurrently,
				// its value can't pass 'child' idx without locking 'child' mutex
				if (child < count_)
					return {child, std::move(lg)};
				else
					return {null, {}};
			}


			const size_t max_count_;
			const double load_factor_;
			std::atomic<idx_t> count_;
			TableNode* disposed_nodes_head_;
			std::mutex disposed_nodes_lock_;

			std::vector<TableNode*> buckets_;
			std::vector<std::mutex> bucket_locks_;
			std::vector<TableNode> table_nodes_;
			std::vector<HeapNode> heap_;
			std::vector<std::mutex> heap_locks_;
		};

		template <bool UseBackoff, bool UseShortIndex = true,
				typename PriorityT = utility::WstPriority,
				typename HasherT = mmh2::MurmurHash2<void>>
		struct ConcurrentHashtableTypeHolder {
			template <typename KeyT, typename ValueT>
			using container_t = ConcurrentHashtable<KeyT, ValueT, PriorityT, HasherT, UseBackoff, UseShortIndex>;
		};
	}
}
#endif //NUMDB_CONCURRENT_HASHTABLE_BINARY_HEAP_H
