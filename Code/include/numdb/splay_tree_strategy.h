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

#endif //NUMDB_SPLAY_TREE_STRATEGY_H
