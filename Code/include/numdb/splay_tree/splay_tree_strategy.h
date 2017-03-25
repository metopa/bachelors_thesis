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
	uint32_t accesses = 0;

	bool shouldSplay(AccessCountSplayStrategy* child) {
		return accesses < child->accesses;
	}
	void visited() {}
	void accessed() {
		accesses = std::max(accesses + 1, std::numeric_limits<uint32_t>::max());
	}

	void dumpStrategy(std::ostream& out) const {
		out << "<accesses: " << accesses << ">";
	}
};

template <unsigned char SCORE_BOOST, unsigned char SCORE_DEGRADATION,
		unsigned char MAX_SCORE, unsigned char INITIAL_SCORE = MAX_SCORE / 2>
struct ParametrizedAccessCountSplayStrategy {
	unsigned char score = INITIAL_SCORE;

	bool shouldSplay(ParametrizedAccessCountSplayStrategy* child) {
		return score < child->score;
	}
	void visited() {
		score = std::max<unsigned char>(score - SCORE_DEGRADATION, 0);
	}
	void accessed() {
		score = std::min<unsigned char>(score + SCORE_BOOST, MAX_SCORE);
	}

	void dumpStrategy(std::ostream& out) const {
		out << "<score: " << score << '/' << MAX_SCORE << ">";
	}
};

#endif //NUMDB_SPLAY_TREE_STRATEGY_H
