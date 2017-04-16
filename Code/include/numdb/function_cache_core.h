/** @file function_cache_core.h
 *  @brief The following class contains common code for all function caches.
 *  @details It is designed to be a class member of another class. Other function classes could be derived from this class, but it brings some problems with typedefs and calling members of the base class (one is forced to specify the base class with all template arguments).
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FUNCTION_CACHE_CORE_H
#define NUMDB_FUNCTION_CACHE_CORE_H

#include <tuple>
#include <chrono>
#include <type_traits>
#include <experimental/tuple> //std::experimental::apply

#include "initial_priority_generator.h"
#include "utils.h"


template <
		typename UserFuncT,
		typename UserFuncArgsTupleT, /*=deduced arguments of UserFuncT*/
		typename EventCounterT>
class FunctionCacheCore {
  public:
	using user_func_t = UserFuncT;
	using args_tuple_t = UserFuncArgsTupleT;
	using return_t =
	decltype(std::experimental::apply(
			std::declval<UserFuncT>(),
			std::declval<args_tuple_t>()));
	using event_counter_t = EventCounterT;

	EventCounterT& getEventCounter() {
		return event_counter_;
	}

	FunctionCacheCore(UserFuncT&& user_func) :
			user_func_(std::move(user_func)) {}

	FunctionCacheCore(const FunctionCacheCore&) = delete;
	FunctionCacheCore& operator =(const FunctionCacheCore&) = delete;

	auto invokeUserFunc(const UserFuncArgsTupleT& args,
						size_t& priority) {
		using namespace std::chrono;
		event_counter_.invokeUserFunc();
		auto start = high_resolution_clock::now();
		DEFERRED(
			priority = priority_generator_.calculatePriority(
					(uint64_t) duration_cast<microseconds>(
							high_resolution_clock::now() - start
					).count());
		);
		return std::experimental::apply(user_func_, args);
	}

  private:
	ProportionPriorityGenerator<256> priority_generator_;
	UserFuncT user_func_;
	EventCounterT event_counter_;
};

#endif //NUMDB_FUNCTION_CACHE_CORE_H

