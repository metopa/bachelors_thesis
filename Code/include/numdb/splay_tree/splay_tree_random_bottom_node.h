/** @file splay_tree_random_bottom_node.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_SPLAY_TREE_RANDOM_BOTTOM_NODE_H
#define NUMDB_SPLAY_TREE_RANDOM_BOTTOM_NODE_H

#include <murmurhash2/all.h>

#include "../utils.h"
#include "splay_tree_base.h"

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
class SplayTreeBottomNode;

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
struct CacheContainerTraits<SplayTreeBottomNode
		<KeyT, ValueT, SplayStrategyT, ComparatorT>> {
	using key_t = KeyT;
	using value_t = ValueT;
	using strategy_t = SplayStrategyT;
	using comparator_t = ComparatorT;
	using node_base_t = Empty;
	static constexpr bool enableRefToSelf() { return false; }
};

template <typename SplayStrategyT, typename ComparatorT = std::less<>>
struct SplayTreeBottomNodeTypeHolder {
	template <typename KeyT, typename ValueT>
	using container_t = SplayTreeBottomNode
			<KeyT, ValueT, SplayStrategyT, ComparatorT>;
};

template <typename KeyT, typename ValueT,
		typename SplayStrategyT, typename ComparatorT>
class SplayTreeBottomNode :
		public SplayTreeBase<SplayTreeBottomNode
				<KeyT, ValueT, SplayStrategyT, ComparatorT>> {
  public:
	using base_t = SplayTreeBase<
			SplayTreeBottomNode<KeyT, ValueT, SplayStrategyT, ComparatorT>>;
	using node_t = typename base_t::Node;

	SplayTreeBottomNode(size_t max_node_count, ComparatorT comparator = {}) :
			base_t(max_node_count, std::move(comparator)) {}

	void nodeAccessedImpl(node_t* node) {}
	void nodeVisitedImpl(node_t* node) {}
	void nodeInsertedImpl(node_t* node) {}
	void nodeExtractedImpl(node_t* node) {}
	node_t** getLuNodeRefImpl(const typename base_t::key_t& key) {
		size_t pseudo_random = mmh2::getMurmurHash2(key);
		node_t** node = &(base_t::root_);
		while (true) {
			if (pseudo_random & 1) {
				if (!(*node)->left_)
					break;
				node = &((*node)->left_);
			} else {
				if (!(*node)->right_)
					break;
				node = &((*node)->right_);
			}
			pseudo_random >>= 1;
		}
		return node;
	}
};

#endif //NUMDB_SPLAY_TREE_RANDOM_BOTTOM_NODE_H
