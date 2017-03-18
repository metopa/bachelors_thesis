/** @file fixed_hashtable_fair_lru.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FIXED_HASHTABLE_FAIR_LRU_H
#define NUMDB_FIXED_HASHTABLE_FAIR_LRU_H

#include "murmurhash2/all.h"

#include "fixed_hashtable_base.h"
#include "utils.h"
#include "fair_lru.h"

template <typename KeyT, typename ValueT, typename HasherT>
class FixedHashtableFairLRU;

template <typename KeyT, typename ValueT, typename HasherT>
struct HashtableTraits<FixedHashtableFairLRU<KeyT, ValueT, HasherT>> {
	typedef KeyT key_t;
	typedef ValueT value_t;
	typedef HasherT hasher_t;
	typedef FairLRU::Node node_base_t;
};

template <typename KeyT, typename ValueT,
		typename HasherT = mmh2::MurmurHash2<KeyT>>
class FixedHashtableFairLRU :
		public FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>> {

  public:
	typedef FixedHashtableBase<
			FixedHashtableFairLRU<KeyT, ValueT, HasherT>
	> base_t;
	typedef typename base_t::Node node_t;

	friend base_t;

	FixedHashtableFairLRU(size_t table_size, size_t max_element_count) :
			base_t(table_size, max_element_count) {}

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

	node_t* extractLruNodeImpl() {
		return static_cast<node_t*>(lru_manager_.extractLruNode());
	}

  private:
	FairLRU lru_manager_;
};

#endif //NUMDB_FIXED_HASHTABLE_FAIR_LRU_H
