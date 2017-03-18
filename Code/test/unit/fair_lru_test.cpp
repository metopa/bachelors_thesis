/** @file fair_lru_test.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */
 

#include <gtest/gtest.h>
#include "numdb/fair_lru.h"

struct Node : public FairLRU::Node {};

TEST(FairLRU, node_insertion_extraction) {
	FairLRU lru;
	std::unique_ptr<Node> a (new Node());
	EXPECT_EQ(lru.extractLruNode(), nullptr);
	lru.insertNode(a.get());
	EXPECT_EQ(lru.extractLruNode(), a.get());
	EXPECT_EQ(lru.extractLruNode(), nullptr);
	EXPECT_EQ(lru.extractLruNode(), nullptr);
	lru.insertNode(a.get());
	EXPECT_EQ(lru.extractLruNode(), a.get());
	EXPECT_EQ(lru.extractLruNode(), nullptr);
	EXPECT_EQ(lru.extractLruNode(), nullptr);
}

TEST(FairLRU, fifo_test) {
	FairLRU lru;
	std::unique_ptr<Node> a(new Node());
	std::unique_ptr<Node> b(new Node());
	std::unique_ptr<Node> c(new Node());

	lru.insertNode(a.get());
	lru.insertNode(b.get());
	lru.insertNode(c.get());
	EXPECT_EQ(lru.extractLruNode(), a.get());
	EXPECT_EQ(lru.extractLruNode(), b.get());
	EXPECT_EQ(lru.extractLruNode(), c.get());
	EXPECT_EQ(lru.extractLruNode(), nullptr);
}

TEST(FairLRU, mark_ru_test) {
	FairLRU lru;
	std::unique_ptr<Node> a(new Node());
	std::unique_ptr<Node> b(new Node());
	std::unique_ptr<Node> c(new Node());

	lru.insertNode(a.get());
	lru.insertNode(b.get());
	lru.insertNode(c.get());
	lru.markRecentlyUsed(a.get());
	lru.markRecentlyUsed(b.get());
	EXPECT_EQ(lru.extractLruNode(), c.get());
	EXPECT_EQ(lru.extractLruNode(), a.get());

	lru.insertNode(a.get());
	lru.markRecentlyUsed(b.get());
	lru.insertNode(c.get());
	lru.markRecentlyUsed(b.get());
	lru.markRecentlyUsed(c.get());
	EXPECT_EQ(lru.extractLruNode(), a.get());
	EXPECT_EQ(lru.extractLruNode(), b.get());
	EXPECT_EQ(lru.extractLruNode(), c.get());
	EXPECT_EQ(lru.extractLruNode(), nullptr);
}
