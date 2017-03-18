/** @file fixed_hashtable.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FIXED_HASHTABLE_H
#define NUMDB_FIXED_HASHTABLE_H

#include <functional>
#include <cmath>
#include <vector>
#include <ostream>
#include <experimental/optional>

#include "murmurhash2/all.h"

//TODO Restrict class to use a power of 2 as a table size
//TODO Use bit masking instead of integer division
//TODO Preallocate all nodes
template <typename KeyT, typename ValueT,
		typename HasherT = mmh2::MurmurHash2<KeyT>>
class FixedHashtable {
  public:
	using optional_value_t = std::experimental::optional<ValueT>;

	class Node {
		friend class FixedHashtable;

	  public:
		Node(KeyT key, ValueT value, Node* next = nullptr) :
				next_(next), key_(std::move(key)), value_(std::move(value)) {}

		~Node() {
			delete next_;
		}

		KeyT& key() { return key_; }
		const KeyT& key() const { return key_; }

		ValueT& value() { return value_; }
		const ValueT& value() const { return value_; }

	  private:
		void extract(Node** this_node_ref) {
			assert(this_node_ref != nullptr && this == *this_node_ref);
			*this_node_ref = next_;
		}

		void insertBefore(Node** next_node_ref) {
			assert(next_node_ref != nullptr);
			next_ = *next_node_ref;
			*next_node_ref = this;
		}

		Node* next_;
		KeyT key_;
		ValueT value_;
	};

	FixedHashtable(size_t table_size, size_t max_element_count) :
			buckets_(table_size, nullptr) {}

	FixedHashtable(const FixedHashtable&) = delete;
	FixedHashtable& operator=(const FixedHashtable&) = delete;

	~FixedHashtable() {
		for (Node* n : buckets_)
			delete n;
	}

	optional_value_t find(const KeyT& key) {
		Node** root_node = buckets_[getBucket(key)];
		Node** node_ref = root_node;

		while (*node_ref) {
			if ((*node_ref)->key_ == key) {
				Node* found_node = *node_ref;

				if (root_node != node_ref) {
					found_node->extract(node_ref);
					found_node->insertBefore(root_node);
				}
				return {found_node->value_};
			}
			node_ref = &((*node_ref)->next_);
		}

		return {};
	}

	size_t getBucket(const KeyT& key) const {
		size_t bucket = HasherT()(key) % buckets_.size();
		return bucket;
	}

	bool insert(KeyT key, ValueT value) {
		Node** root_node = &buckets_[getBucket(key)];
		Node** node = root_node;
		while (*node) {
			if ((*node)->key_ == key)
				return false;
			node = &((*node)->next_);
		}
		*root_node = new Node(std::move(key), std::move(value), *root_node);
		return true;
	}

	Node* extractNode(const KeyT& key) {
		Node** node_ref = &buckets_[getBucket(key)];

		while (*node_ref) {
			if ((*node_ref)->key_ == key) {
				Node* found_node = *node_ref;
				found_node->extract(node_ref);
				return found_node;
			}
			node_ref = &((*node_ref)->next_);
		}

		return nullptr;
	}

	bool remove(const KeyT& key) {
		Node* n = extractNode(key);
		if (!n)
			return false;
		delete n;
		return true;
	}

	static constexpr size_t maxElemCountForCapacity(size_t capacity, double load_factor = 2) {
		return static_cast<size_t>(capacity / (sizeof(Node) + sizeof(Node*) / load_factor));
	}

	void dump(std::ostream& out) {
		for (size_t i = 0; i < buckets_.size(); i++) {
			out << '[' << i << ']';
			Node* node = buckets_[i];
			while (node) {
				out << "->" << node->key_ << '[' << node->value_ << ']';
				node = node->next_;
			}
			out << std::endl;
		}
	}

  private:
	std::vector<Node*> buckets_;
};

#endif //NUMDB_FIXED_HASHTABLE_H
