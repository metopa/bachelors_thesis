/** @file splay_tree_fair_lru.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_SPLAY_TREE_FAIR_LRU_H
#define NUMDB_SPLAY_TREE_FAIR_LRU_H

#include "../fair_lru.h"
#include "../utils.h"
#include "splay_tree_base.h"

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
class SplayTreeFairLRU;

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
struct CacheContainerTraits<SplayTreeFairLRU
		<KeyT, ValueT, SplayStrategyT, ComparatorT>> {
	using key_t = KeyT;
	using value_t = ValueT;
	using strategy_t = SplayStrategyT;
	using comparator_t = ComparatorT;
	using node_base_t = FairLRU::Node;
	static constexpr bool enableRefToSelf() { return true; }
};

template <typename SplayStrategyT, typename ComparatorT = std::less<>>
struct SplayTreeFairLRUTypeHolder {
	template <typename KeyT, typename ValueT>
	using container_t = SplayTreeFairLRU<KeyT, ValueT,
			SplayStrategyT, ComparatorT>;
};

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
class SplayTreeFairLRU : public SplayTreeBase<
		SplayTreeFairLRU<KeyT, ValueT, SplayStrategyT, ComparatorT>> {
  public:
	using base_t = SplayTreeBase<
			SplayTreeFairLRU<KeyT, ValueT, SplayStrategyT, ComparatorT>>;
	using node_t = typename base_t::Node;

	SplayTreeFairLRU(size_t max_node_count, ComparatorT comparator = {}) :
			base_t(max_node_count, std::move(comparator)) {}

	void nodeAccessedImpl(node_t* node) {
		lru_manager_.markRecentlyUsed(node);
	}
	void nodeVisitedImpl(node_t* node) {}
	void nodeInsertedImpl(node_t* node) {
		lru_manager_.insertNode(node);
	}
	void nodeExtractedImpl(node_t* node) {
		lru_manager_.extractNode(node);
	}
	node_t** getLruNodeRefImpl(const typename base_t::key_t& /*key*/) {
		node_t* node = static_cast<node_t*>(lru_manager_.extractLruNode());
		assert(node != nullptr);
		assert(node->hasRefToSelf());
		assert(node->getRefToSelf() != nullptr);
		return node->getRefToSelf();
	}

  private:
	FairLRU lru_manager_;
};

#endif //NUMDB_SPLAY_TREE_FAIR_LRU_H
