/** @file fair_lru.h
 *  @brief
 *  @warning FairLRU does not manages a lifetime of it's nodes. Node struct is designed to be a base class of another node, defined in a container and this contained must control a lifetime of it's nodes.
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FAIR_LRU_H
#define NUMDB_FAIR_LRU_H

class FairLRU {
  public:
	struct Node {
		friend class FairLRU;

		Node* next;
		Node** this_in_prev;

		Node() : next(nullptr), this_in_prev(nullptr) {}

	  private:
		Node* extract() {
			*this_in_prev = next;
			if (next)
				next->this_in_prev = this_in_prev;
			next = nullptr;
			return this;
		}

		void insertBefore(Node** node_ref) {
			next = *node_ref;
			this_in_prev = node_ref;
			*node_ref = this;
			if (next)
				next->this_in_prev = &(next);
		}

		void insertBefore(Node* node) {
			insertBefore(node->this_in_prev);
		}
	};

	FairLRU() {
		head_ = tail_ = new Node();
		tail_->this_in_prev = &head_;
	}

	~FairLRU() {
		delete tail_;
	}

	FairLRU(const FairLRU&) = delete;
	FairLRU& operator =(const FairLRU&) = delete;

	Node* extractLruNode() {
		if (head_ == tail_)
			return nullptr;
		return head_->extract();
	}

	void markRecentlyUsed(Node* node) {
		node->extract();
		insertNode(node);
	}

	void insertNode(Node* node) {
		node->insertBefore(tail_);
	}

	void extractNode(Node* node) {
		node->extract();
	}
  private:
	Node* head_;
	Node* tail_;
};

#endif //NUMDB_FAIR_LRU_H
