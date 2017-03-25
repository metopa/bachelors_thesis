#include <gtest/gtest.h>
#include <numdb/splay_tree/splay_tree_fair_lru.h>
#include <numdb/splay_tree/splay_tree_strategy.h>

/** @file splay_tree.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */
 
TEST(SplayTree, basic) {
	using container_t = typename SplayTreeFairLRUTypeHolder
			<CanonicalSplayStrategy>::template container_t<int, int>;
	container_t tree(container_t::elementSize() * 8);
	tree.insert(5, 5);
	auto result = tree.find(5);
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, 5);
	tree.remove(5);
	result = tree.find(5);
	ASSERT_EQ(bool(result), false);
}
