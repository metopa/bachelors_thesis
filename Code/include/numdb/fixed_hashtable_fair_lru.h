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

template <typename KeyT, typename ValueT,
		typename HasherT = mmh2::MurmurHash2<KeyT>>
class FixedHashtableFairLRU : public FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>, Empty, KeyT, ValueT, HasherT> {
  public:
	FixedHashtableFairLRU(size_t table_size, size_t max_element_count) :
			FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>, Empty, KeyT, ValueT, HasherT> (table_size, max_element_count) {}

  protected:
	void nodeAccessedImpl(typename FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>, Empty, KeyT, ValueT, HasherT>::Node* node)  {

	}
	typename FixedHashtableBase<FixedHashtableFairLRU<KeyT, ValueT, HasherT>, Empty, KeyT, ValueT, HasherT>::Node* extractLruNodeImpl() {
		return nullptr;
	}
};

#endif //NUMDB_FIXED_HASHTABLE_FAIR_LRU_H
