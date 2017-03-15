/** @file function_cache_core.h
 *  @brief The following class contains common code for all function caches.
 *  @details It is designed to be a class member of another class. Other function classes could be derived from this class, but it brings some problems with typedefs and calling members of the base class (one is forced to specify the base class with all template arguments).
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FUNCTION_CACHE_CORE_H
#define NUMDB_FUNCTION_CACHE_CORE_H

#include <tuple>
#include <type_traits>
#include <experimental/tuple> //std::experimental::apply


template <
		typename UserFuncT,
		typename UserFuncArgsTupleT, /*=deduced arguments of UserFuncT*/
		typename EventCounterT>
class FunctionCacheCore {
  public:
	using user_func_t = UserFuncT;
	using return_t = std::result_of<UserFuncT>;
	using args_tuple_t = UserFuncArgsTupleT;
	using event_counter_t = EventCounterT;

	EventCounterT& getEventCounter() {
		return event_counter_;
	}

	FunctionCacheCore(UserFuncT&& user_func) :
			user_func_(std::move(user_func)) {}

	FunctionCacheCore(const FunctionCacheCore&) = delete;
	FunctionCacheCore& operator =(const FunctionCacheCore&) = delete;

	auto invokeUserFunc(UserFuncArgsTupleT&& args) {
		event_counter_.retrieve();
		return std::experimental::apply(user_func_, std::move(args));
	}

  private:
	UserFuncT user_func_;
	EventCounterT event_counter_;
};

#endif //NUMDB_FUNCTION_CACHE_CORE_H

