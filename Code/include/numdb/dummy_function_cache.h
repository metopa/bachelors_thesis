/** @file dummy_engine.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_DUMMY_FUNCTION_CACHE_H
#define NUMDB_DUMMY_FUNCTION_CACHE_H

#include <tuple>
#include <type_traits>

#include <function_traits/functional_unwrap.hpp>

#include "function_cache_core.h"
#include "event_counter.h"
#include "utils.h"

template <
		typename UserFuncT,
		typename EventCounterT = EmptyEventCounter,
		typename UserFuncArgsTupleT = fu::argument_tuple_type_of_t<UserFuncT>
>
class DummyFunctionCache {
	using core_t = FunctionCacheCore<UserFuncT,
			UserFuncArgsTupleT, EventCounterT>;
  public:
	static constexpr bool is_threadsafe() { return false; }

	DummyFunctionCache(UserFuncT user_func,
					   size_t /*available_memory*/ = 0) :
			core_(std::move(user_func)) {}

	template <typename... Args>
	auto operator ()(Args&& ... args) {
		static_assert(std::is_convertible<std::tuple<Args...>, typename core_t::args_tuple_t>::value,
					  "Cannot convert provided arguments.");

		core_.getEventCounter().invokeUserFunc();
		return core_.invokeUserFunc(std::forward_as_tuple(args...));
	}
  private:
	core_t core_;
};

#endif //NUMDB_DUMMY_FUNCTION_CACHE_H
