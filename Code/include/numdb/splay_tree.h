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

//TODO Allocate memory in a single block
//TODO Track inserted objects
//TODO Use strategy when splaying
//TODO Choose unused node
template <typename KeyT, typename ValueT,
		typename SplayStrategyT,
		typename ComparatorT = std::less<KeyT>>
class SplayTree {
  public:
	//Empty member optimization http://www.cantrip.org/emptyopt.html
	class Node : SplayStrategyT {
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
				out << node->key_ << "->" << node->value_ << std::endl;
				dump(node->left_, out, level + 1);
				dump(node->right_, out, level + 1);
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

	Node* root_;
	size_t node_count_;
	const size_t max_node_count_;

	using optional_value_t = std::experimental::optional<ValueT>;

	SplayTree(size_t max_node_count) :
			root_(nullptr), node_count_(0),
			max_node_count_(max_node_count) {}

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
		LEFT, RIGHT, UNDEFINED
	};

	Node*& findImpl(const KeyT& key, Node*& node) {
		if (!node)
			return node;
		if (key == node->key_)
			return node;

		if (key < node->key_)
			return findImpl(key, node->left_);
		else
			return findImpl(key, node->right_);
	}

	Node* findImplSplay(const KeyT& key, Node*& node,
						EChildType& child_type, bool is_root) {
		if (!node)
			return node;
		if (key == node->key_)
			return node;

		Node* result;
		EChildType grandchild = EChildType::UNDEFINED;

		if (key < node->key_) {
			child_type = EChildType::LEFT;
			result = findImplSplay(key, node->left_, grandchild, false);
		} else {
			child_type = EChildType::RIGHT;
			result = findImplSplay(key, node->right_, grandchild, false);
		}
		//TODO Add strategy
		if (splay(node, child_type, grandchild, is_root))
			child_type = EChildType::UNDEFINED;
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

	/*
	 * Not a classical splay, in case of zigzag and zigzig operations it splits the operation
	 * */
	bool splay(Node*& node, EChildType parent_grandparent, EChildType child_parent, bool is_root) {
		switch (parent_grandparent) {
			case EChildType::LEFT:
				switch (child_parent) {
					case EChildType::LEFT:
						rotateRight(node);
						rotateRight(node);
						return true;
					case EChildType::RIGHT:
						rotateLeft(node->left_);
						rotateRight(node);
						return true;
					case EChildType::UNDEFINED:
						if (is_root) {
							rotateRight(node);
							return true;
						} else
							return false;
				}

			case EChildType::RIGHT:
				switch (child_parent) {
					case EChildType::LEFT:
						rotateRight(node->right_);
						rotateLeft(node);
						return true;
					case EChildType::RIGHT:
						rotateLeft(node);
						rotateLeft(node);
						return true;
					case EChildType::UNDEFINED:
						if (is_root) {
							rotateLeft(node);
							return true;
						} else
							return false;
				}
			case EChildType::UNDEFINED:
				return false;
		}
	}

	Node*& getPredecessor(Node*& node) {
		Node** pred = &(node->left_);
		while ((*pred)->right)
			pred = &(node->right_);
		return *pred;
	}
};

#endif //NUMDB_SPLAY_TREE_H
