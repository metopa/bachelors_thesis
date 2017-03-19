/** @file fixed_hashtable_fair_lru.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FIXED_HASHTABLE_FAIR_LRU_H
#define NUMDB_FIXED_HASHTABLE_FAIR_LRU_H

#include "murmurhash2/all.h"

#include "fixed_hashtable_base.h"
#include "numdb/utils.h"
#include "numdb/fair_lru.h"

template <typename KeyT, typename ValueT, typename HasherT>
class FixedHashtableFairLRU;

template <typename KeyT, typename ValueT, typename HasherT>
struct CacheContainerTraits<FixedHashtableFairLRU<KeyT, ValueT, HasherT>> {
	using key_t = KeyT;
	using value_t = ValueT;
	using hasher_t = HasherT;
	using node_base_t = FairLRU::Node;
};

template <typename KeyT, typename ValueT,
		typename HasherT = mmh2::MurmurHash2<KeyT>>
class FixedHashtableFairLRU :
		public FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>> {

  public:
	using base_t = FixedHashtableBase<
			FixedHashtableFairLRU<KeyT, ValueT, HasherT>>;
	using node_t = typename base_t::Node;

	friend base_t;

	FixedHashtableFairLRU(size_t available_memory, double load_factor) :
			base_t(available_memory, load_factor) {}

  protected:
	void nodeAccessedImpl(node_t* node) {
		lru_manager_.markRecentlyUsed(node);
	}
	void nodeInsertedImpl(node_t* node) {
		lru_manager_.insertNode(node);
	}

	void nodeExtractedImpl(node_t* node) {
		lru_manager_.extractNode(node);
	}

	node_t* getLruNodeImpl() {
		return static_cast<node_t*>(lru_manager_.extractLruNode());
	}

  private:
	FairLRU lru_manager_;
};

#endif //NUMDB_FIXED_HASHTABLE_FAIR_LRU_H
