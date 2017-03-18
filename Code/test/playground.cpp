/** @file playground.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <iostream>
#include <random>
#include <numdb/fixed_hashtable.h>
#include "numdb/splay_tree.h"
#include "benchmark/utils.h"
#include <unordered_set>

void testSplayTree() {
	SplayTree<
			int, std::string,
			ParametrizedAccessCountSplayStrategy<2, 1, 5>
	> tree(100);
	tree.insert(20, "a");
	tree.insert(18, "b");
	tree.insert(1, "c");
	tree.insert(4, "d");
	tree.insert(12, "e");
	tree.insert(8, "f");
	tree.insert(10, "g");
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(10) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(1) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(18) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(18) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(18) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(10) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(1) << std::endl;
	tree.dump(std::cout);
	std::cout << "\nfind->:" << *tree.find(1) << std::endl;
	tree.dump(std::cout);
}

void testHashTable() {
	unsigned int size = 320;
	FixedHashtable<int, std::string> ht(size, size * 2);
	for (int i = 100; i < 100 + size * 2; i++)
		ht.insert(i * i, "");
	ht.dump(std::cout);
}

int main() {
	testHashTable();
	return 0;
}
