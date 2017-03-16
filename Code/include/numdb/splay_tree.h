/** @file splay_tree.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_SPLAY_TREE_H
#define NUMDB_SPLAY_TREE_H

#include <functional>
#include <experimental/optional>
#include <cassert>
#include <ostream>


struct CanonicalSplayStrategy {
	bool shouldSplay(CanonicalSplayStrategy* child) {
		return true;
	}

	void visited() {}
	void accessed() {}
	void dumpStrategy(std::ostream& out) const {}
};

//TODO Allocate memory in a single block
//TODO Track inserted objects
//TODO Add removal strategy
//TODO Add timestamp
//TODO Add check for splay strategy methods
template <typename KeyT, typename ValueT,
		typename SplayStrategyT,
		typename ComparatorT = std::less<KeyT>>
class SplayTree {
  public:
	//Empty member optimization http://www.cantrip.org/emptyopt.html
	class Node : public SplayStrategyT {
		friend class SplayTree;

		Node(KeyT key, ValueT value,
			 Node* left = nullptr, Node* right = nullptr,
			 SplayStrategyT strategy = SplayStrategyT()) :
				SplayStrategyT(strategy),
				key_(std::move(key)),
				value_(std::move(value)),
				left_(left), right_(right) {}

		const KeyT& key() const {
			return key_;
		}

		KeyT& key() {
			return key_;
		}

		const ValueT& value() const {
			return value_;
		}

		ValueT& value() {
			return value_;
		}

	  private:
		/// Replacement for a destructor, since we don't always want
		/// to delete all ancestors along with the node
		void deleteWithAncestors() {
			if (left_)
				left_->deleteWithAncestors();

			if (right_)
				right_->deleteWithAncestors();

			delete this;
		}

		static void dump(const Node* node, std::ostream& out, int level) {
			for (int i = 0; i < level; i++)
				out << "- ";
			if (!node)
				out << "@" << std::endl;
			else {
				out << node->key_ << "->" << node->value_ << " ";
				node->dumpStrategy(out);
				out << std::endl;
				if (node->left_ || node->right_) {
					dump(node->left_, out, level + 1);
					dump(node->right_, out, level + 1);
				}
			}
		}

	  private:
		// This ordering is intended to keep
		// the most frequently accessed members
		// at the beginning of the object
		KeyT key_;
		Node* left_;
		Node* right_;
		ValueT value_;
	};

	using optional_value_t = std::experimental::optional<ValueT>;

	SplayTree(size_t max_node_count, ComparatorT comparator = ComparatorT()) :
			root_(nullptr), node_count_(0),
			max_node_count_(max_node_count),
			comparator_(std::move(comparator)) {}

	SplayTree(const SplayTree&) = delete;
	SplayTree& operator =(const SplayTree&) = delete;

	~SplayTree() {
		if (root_)
			root_->deleteWithAncestors();
	}

	static constexpr size_t maxElemCountForCapacity(size_t capacity) {
		return capacity / sizeof(Node);
	}

	optional_value_t find(const KeyT& key) {
		EChildType tmp;
		Node* node = findImplSplay(key, root_, tmp, true);
		if (!node)
			return {};
		else
			return {node->value_};
	}

	/*
	 * This approach because we assume that an important node is not going to be deleted.
	 */
	Node* extractNode(const KeyT& key) {
		return extractNode(findImpl(key, root_));
	}

	Node* extractNode(Node*& node_ref) {
		Node* node = node_ref;
		if (!node)
			return nullptr;

		if (!node->left_) {
			node_ref = node->right_; //Can be nullptr
			node->right_ = nullptr;
			return node;
		}
		if (!node->right_) {
			node_ref = node->left_;
			node->left_ = nullptr;
			return node;
		}

		Node* predecessor = extractNode(getPredecessor(node));
		assert(predecessor != nullptr);

		predecessor->left_ = node->left_;
		predecessor->right_ = node->right_;
		node_ref = predecessor;
		node->right_ = node->left_ = nullptr;
		return node;
	}

	bool insert(KeyT key, ValueT value) {
		return insertNode(new Node(std::move(key), std::move(value)));
	}

	bool insertNode(Node* node) {
		Node*& place_to_insert = findImpl(node->key_, root_);
		if (place_to_insert)
			return false;
		place_to_insert = node;
		return true;
	}

	void dump(std::ostream& out) {
		Node::dump(root_, out, 0);
	}

  private:
	enum class EChildType {
		LEFT, RIGHT, UNDEFINED, DONT_SPLAY
	};

	Node*& findImpl(const KeyT& key, Node*& node) {
		if (!node)
			return node;
		if (comparator_(key, node->key_))
			return findImpl(key, node->left_);
		else if (comparator_(node->key_, key))
			return findImpl(key, node->right_);

		else /*key == node->key_*/
			return node;
	}

	Node* findImplSplay(const KeyT& key, Node*& node,
						EChildType& child_type, bool is_root) {
		if (!node)
			return nullptr;

		Node* result;
		EChildType grandchild = EChildType::UNDEFINED;

		if (comparator_(key, node->key_)) {
			node->visited();
			child_type = EChildType::LEFT;
			result = findImplSplay(key, node->left_, grandchild, false);
		} else if (comparator_(node->key_, key)) {
			node->visited();
			child_type = EChildType::RIGHT;
			result = findImplSplay(key, node->right_, grandchild, false);
		} else /*key == node->key_*/ {
			node->accessed();
			return node;
		}

		child_type = splay(node, is_root, child_type, grandchild);

		return result;
	}

	void rotateLeft(Node*& parent_ref) {
		Node* parent = parent_ref;
		assert(parent != nullptr);
		Node* child = parent->right_;
		assert(child != nullptr);
		parent_ref = child;
		parent->right_ = child->left_;
		child->left_ = parent;
	}

	void rotateRight(Node*& parent_ref) {
		Node* parent = parent_ref;
		assert(parent != nullptr);
		Node* child = parent->left_;
		assert(child != nullptr);
		parent_ref = child;
		parent->left_ = child->right_;
		child->right_ = parent;
	}

	EChildType splay(Node*& node, bool is_root,
					 EChildType parent_grandparent,
					 EChildType child_parent) {
		if (child_parent == EChildType::DONT_SPLAY ||
			parent_grandparent == EChildType::DONT_SPLAY)
			return EChildType::DONT_SPLAY;

		switch (parent_grandparent) {
			case EChildType::LEFT:
				if (!node->shouldSplay(node->left_)) {
					splay(node->left_, true, child_parent,
						  EChildType::UNDEFINED);
					return EChildType::DONT_SPLAY;
				}
				switch (child_parent) {
					case EChildType::LEFT:
						rotateRight(node);
						rotateRight(node);
						return EChildType::UNDEFINED;
					case EChildType::RIGHT:
						rotateLeft(node->left_);
						rotateRight(node);
						return EChildType::UNDEFINED;

					case EChildType::UNDEFINED:
						if (is_root) {
							rotateRight(node);
							return EChildType::UNDEFINED;
						} else
							return parent_grandparent;
				}

			case EChildType::RIGHT:
				if (!node->shouldSplay(node->right_)) {
					splay(node->right_, true, child_parent,
						  EChildType::UNDEFINED);
					return EChildType::DONT_SPLAY;
				}
				switch (child_parent) {
					case EChildType::LEFT:
						rotateRight(node->right_);
						rotateLeft(node);
						return EChildType::UNDEFINED;
					case EChildType::RIGHT:
						rotateLeft(node);
						rotateLeft(node);
						return EChildType::UNDEFINED;
					case EChildType::UNDEFINED:
						if (!node->shouldSplay(node->right_))
							return EChildType::DONT_SPLAY;

						if (is_root) {
							rotateLeft(node);
							return EChildType::UNDEFINED;
						} else
							return parent_grandparent;
				}
			case EChildType::UNDEFINED:
				return EChildType::UNDEFINED;
		}
	}

	Node*& getPredecessor(Node*& node) {
		Node** pred = &(node->left_);
		while ((*pred)->right_)
			pred = &(node->right_);
		return *pred;
	}

	Node* root_;
	size_t node_count_;
	const size_t max_node_count_;
	ComparatorT comparator_;
};

#endif //NUMDB_SPLAY_TREE_H
