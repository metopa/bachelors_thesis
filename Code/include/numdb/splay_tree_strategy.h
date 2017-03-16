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
	int accesses = 0;

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
};

template <int SCORE_BOOST, int SCORE_DEGRADATION,
		int MAX_SCORE, int INITIAL_SCORE = MAX_SCORE / 2>
struct ParametrizedAccessCountSplayStrategy {
	int score = INITIAL_SCORE;

	bool shouldSplay(ParametrizedAccessCountSplayStrategy* child) {
		return score < child->score;
	}
	void visited() {
		score = std::max(score - SCORE_DEGRADATION, 0);
	}
	void accessed() {
		score = std::min(score + SCORE_BOOST, MAX_SCORE);
	}

	void dumpStrategy(std::ostream& out) const {
		out << "<score: " << score << '/' << MAX_SCORE << ">";
	}
};

#endif //NUMDB_SPLAY_TREE_STRATEGY_H
