/** @file fixed_hashtable_base.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FIXED_HASHTABLE_BASE_H
#define NUMDB_FIXED_HASHTABLE_BASE_H

#include <functional>
#include <cmath>
#include <vector>
#include <ostream>
#include <experimental/optional>


//TODO Restrict class to use a power of 2 as a table size
//TODO Use bit masking instead of integer division
//TODO Preallocate all nodes
template <typename CrtpDerived, typename NodeBaseClass,
		typename KeyT, typename ValueT,
		typename HasherT>
class FixedHashtableBase {
  public:
	using optional_value_t = std::experimental::optional<ValueT>;

	class Node : public NodeBaseClass {
		friend class FixedHashtableBase;

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

	FixedHashtableBase(size_t table_size, size_t max_element_count) :
			buckets_(table_size, nullptr) {}

	FixedHashtableBase(const FixedHashtableBase&) = delete;
	FixedHashtableBase& operator =(const FixedHashtableBase&) = delete;

	~FixedHashtableBase() {
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
		return static_cast<size_t>(
				capacity / (sizeof(Node) + sizeof(Node*) / load_factor));
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
	void nodeAccessed(Node* node) {
		static_cast<CrtpDerived>(this)->nodeAccessedImpl(node);
	}

	Node* extractLruNode() {
		return static_cast<CrtpDerived>(this)->extractLruNodeImpl();
	}

  protected:
	///Following methods would be implemented in derived classes.
	///They are called through Curiously Recurring Template Pattern
	void nodeAccessedImpl(Node* node);
	Node* extractLruNodeImpl();
  private:
	std::vector<Node*> buckets_;
};

#endif //NUMDB_FIXED_HASHTABLE_BASE_H
