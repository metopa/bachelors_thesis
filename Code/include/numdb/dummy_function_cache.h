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

template<typename EventCounterT,
		typename UserFuncT,
		typename... UserFuncArgsT>
class DummyFunctionCache :
		FunctionCache<false, EventCounterT, UserFuncT, UserFuncArgsT...> {
  public:
	typedef UserFuncT user_func_t;
	typedef std::result_of<UserFuncT> return_t;
	typedef std::tuple<UserFuncArgsT> args_tuple_t;

	DummyFunctionCache(user_func_t user_func, size_t /*available_memory*/ = 0) :
			FunctionCache(std::move(user_func)) {}

	return_t operator() (UserFuncArgsT&& ... args) {
		argsConvertibleCheck<UserFuncArgsT...>();
		getEventCounter().total_retrieves++;
		return invokeUserFunc(std::forward_as_tuple(args...));
	}

};

#endif //NUMDB_DUMMY_FUNCTION_CACHE_H
