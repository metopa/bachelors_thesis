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

TEST(HashTable_FairLRU, tuple_key) {
	FixedHashtableFairLRU<std::tuple<double, double, double>,
			std::string> ht(10, 20);
	ht.insert(std::make_tuple(0., 0., 0.), "000");
	ht.insert(std::make_tuple(1., 0., 0.), "100");
	ht.insert(std::make_tuple(0., 1., 0.), "010");
	ht.insert(std::make_tuple(0., 0., 1.), "001");
	auto result = ht.find(std::make_tuple(-0., -0., -0.));
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, "000");

	result = ht.find(std::make_tuple(1., 0., 0.));
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, "100");

	result = ht.find(std::make_tuple(0., 1, 0.));
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, "010");

	result = ht.find(std::make_tuple(0., 0., 1));
	ASSERT_EQ(bool(result), true);
	ASSERT_EQ(*result, "001");

	result = ht.find(std::make_tuple(0., -1, 0.));
	ASSERT_EQ(bool(result), false);
}

TEST(HashTable_FairLRU, limit_element_count_1) {
	FixedHashtableFairLRU<int, int> ht(5, 10);
	for (int i = 10; i < 25; i++)
		ht.insert(i, i);

	for (int i = 10; i < 15; i++) {
		auto result = ht.find(i);
		ASSERT_EQ(bool(result), false);
	}

	for (int i = 15; i < 25; i++) {
		auto result = ht.find(i);
		ASSERT_EQ(bool(result), true);
		ASSERT_EQ(*result, i);
	}
}
