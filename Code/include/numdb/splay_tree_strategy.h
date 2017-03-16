/** @file splay_tree_strategy.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_SPLAY_TREE_STRATEGY_H
#define NUMDB_SPLAY_TREE_STRATEGY_H

#include <ostream>

struct CanonicalSplayStrategy {
	bool shouldSplay(CanonicalSplayStrategy* child) {
		return true;
	}

	void visited() {}
	void accessed() {}
	void dumpStrategy(std::ostream& out) const {}
};

struct AccessCountSplayStrategy {
	bool shouldSplay(AccessCountSplayStrategy* child) {
		return accesses < child->accesses;
	}

	void visited() {}
	void accessed() {
		accesses++;
	}
	void dumpStrategy(std::ostream& out) const {
		out << "<accesses: " << accesses << ">";
	}

	int accesses = 0;
};

#endif //NUMDB_SPLAY_TREE_STRATEGY_H
