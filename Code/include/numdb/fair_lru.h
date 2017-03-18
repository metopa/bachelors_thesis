/** @file fair_lru.h
 *  @brief
 *  @warning FairLRU does not manages a lifetime of it's nodes. Node class is designed to be a base class of another node, defined in a container and this contained must control a lifetime of it's nodes.
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FAIR_LRU_H
#define NUMDB_FAIR_LRU_H

class FairLRU {
  public:
	struct Node {
		friend class FairLRU;

		Node* next;
		Node* prev;

		Node() : next(nullptr), prev(nullptr) {}

	  protected:
		/**
		 * @remark Destructor is declared protected so only derived node can be deleted. With this limitation, dtor may not be declared virtual. This way we eliminate a vtable cost.
		*/
		~Node() = default;

	  private:
		Node* extract() {
			prev->next = next;
			if (next)
				next->prev = prev;
			return this;
		}

		void insertAfter(Node* node) {
			prev = node;
			next = node->next;
			node->next = this;
			if (next)
				next->prev = this;
		}
	};

	FairLRU() {
		tail_.insertAfter(&head_);
	}

	FairLRU(const FairLRU&) = delete;
	FairLRU& operator =(const FairLRU&) = delete;

	Node* extractLruNode() {
		if (head_.next == &tail_)
			return nullptr;
		return head_.next->extract();
	}

	void markRecentlyUsed(Node* node) {
		node->extract();
		insertNode(node);
	}

	void insertNode(Node* node) {
		node->insertAfter(tail_.prev);
	}

	void extractNode(Node* node) {
		node->extract();
	}

  private:
	Node head_;
	Node tail_;
};

#endif //NUMDB_FAIR_LRU_H
