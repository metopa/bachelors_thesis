/** @file fair_lfu.h
 *  @brief
 *  @warning FairLFU does not manages a lifetime of it's nodes. Node class is designed to be a base class of another node, defined in a container and this contained must control a lifetime of it's nodes.
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FAIR_LFU_H
#define NUMDB_FAIR_LFU_H

#include <cstdint>
#include <cassert>

namespace numdb {
	namespace utility {
		class FairLFU {
		  public:
			struct Node {
				friend class FairLFU;

			  protected:
				/**
				 * @remark Destructor is declared protected so only derived node can be deleted. With this limitation, dtor may not be declared virtual. This way we eliminate a vtable cost.
				*/
				~Node() = default;

			  private:
				bool inFreqList() const {
					return group_next != nullptr;
				}

				uint64_t access_frequency = 0;
				Node* next = nullptr;
				Node* prev = nullptr;
				Node* group_next = nullptr;
			};

			FairLFU() {
				head_.group_next = &head_;
			}
			FairLFU(const FairLFU&) = delete;
			FairLFU& operator =(const FairLFU&) = delete;

			Node* extractLuNode() {
				assert(head_.next);
				Node* node = head_.next;
				extractNode(node);
				return node;
			}

			void markRecentlyUsed(Node* node) {
				assert(node);
				node->access_frequency++;
				if (node->group_next == node &&
					(!node->next ||
					 node->access_frequency < node->next->access_frequency))
					return;
				extractNode(node);
				if (node->group_next)
					insertAfter(node->group_next, node);
				else {
					Node* prev = node->prev;
					while (!prev->group_next) {
						assert(prev->prev);
						prev = prev->prev;
					}
					insertAfter(prev, node);
				}

			}

			void insertNode(Node* node) {
				Node* prev = head_.next ? head_.next : &head_;

				node->access_frequency = prev->access_frequency + 1;
				insertAfter(prev, node);
			}

			void extractNode(Node* node) {
				assert(node != nullptr);
				if (node->inFreqList()) {
					if (node->group_next == node) {
						if (node->prev) {
							assert(node->prev->next == node);
							assert(node->prev->access_frequency < node->access_frequency);
							node->prev->next = node->next;
						}
						if (node->next) {
							assert(node->next->prev == node);
							node->next->prev = node->prev;
						}
					} else {
						// promote next node in group as a group head
						Node* new_head = node->group_next;
						new_head->group_next = new_head->next ? new_head->next : new_head;
						new_head->prev = node->prev;
						new_head->next = node->next;
						if (node->prev) {
							assert(node->prev->next == node);
							node->prev->next = new_head;
						}
						if (node->next) {
							assert(node->next->prev == node);
							node->next->prev = new_head;
						}
					}
				} else { //Node is in a group list
					assert(node->prev != nullptr);
					if (node->prev->inFreqList())
						node->prev->group_next = node->next ? node->next : node->prev;
						// freq list node points to itself
						// when is a single node in the group
					else {
						assert(node->prev->next == node);
						node->prev->next = node->next;
					}
					if (node->next) {
						assert(node->next->prev == node);
						node->next->prev = node->prev;
					}
				}
			}

		  private:
			void insertAfter(Node* prev, Node* node) {
				assert(prev);
				assert(prev->inFreqList());
				assert(node);
				assert(prev->access_frequency <= node->access_frequency);
				if (prev->access_frequency == node->access_frequency) {
					node->group_next = nullptr;
					node->prev = prev;
					node->next = prev->group_next == prev ? nullptr : prev->group_next;
					prev->group_next = node;
					if (node->next)
						node->next->prev = node;
				} else if (!prev->next ||
						   prev->next->access_frequency > node->access_frequency) {
					node->group_next = node;
					node->next = prev->next;
					node->prev = prev;
					prev->next = node;
					if (node->next) {
						assert(node->next->prev == prev);
						node->next->prev = node;
					}
				} else
					insertAfter(prev->next, node);
			}

			Node head_;
		};
	}
}
#endif //NUMDB_FAIR_LFU_H
