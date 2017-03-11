/** @file dummy_engine.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_DUMMY_FUNCTION_CACHE_H
#define NUMDB_DUMMY_FUNCTION_CACHE_H

#include <tuple>
#include <type_traits>

#include "numdb.h"
#include "event_counter.h"

template<
		typename UserFuncT,
		typename UserFuncArgsTupleT,
		typename EventCounterT = EmptyEventCounter
>
class DummyFunctionCache :
		FunctionCache<false, UserFuncT, UserFuncArgsTupleT, EventCounterT> {
  public:
	DummyFunctionCache(user_func_t user_func, size_t /*available_memory*/ = 0) :
			FunctionCache(std::move(user_func)) {}

	return_t operator ()(UserFuncArgsTupleT&& ... args) {
		argsConvertibleCheck<UserFuncArgsTupleT...>();
		getEventCounter().invokeUserFunc();
		return invokeUserFunc(std::forward_as_tuple(args...));
	}

};

#endif //NUMDB_DUMMY_FUNCTION_CACHE_H
