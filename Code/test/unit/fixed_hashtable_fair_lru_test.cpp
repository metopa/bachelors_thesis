/** @file fixed_hashtable_fair_lru_test.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */


#include <gtest/gtest.h>
#include "numdb/fixed_hashtable_fair_lru.h"

TEST(HashTable_FairLRU, basic) {
	FixedHashtableFairLRU<int, std::string> ht(10, 20);
	ht.insert(10, "AAA");
	auto result = ht.find(10);
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, "AAA");
	result = ht.find(30);
	ASSERT_EQ(bool(result), false);
}
