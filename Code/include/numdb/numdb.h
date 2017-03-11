/** @file numdb.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_NUMDB_H
#define NUMDB_NUMDB_H

#include <tuple>
#include <type_traits>
#include <experimental/tuple> //std::experimental::apply

//TODO Deduce a default value for UserFuncArgTupleT
template<typename EventCounterT,
		typename UserFuncT,
		typename UserFuncArgsTupleT /*=deduced arguments of UserFuncT*/>
class FunctionCache {
  public:
	typedef UserFuncT user_func_t;
	typedef std::result_of<UserFuncT> return_t;
	typedef UserFuncArgsTupleT args_tuple_t;
	typedef EventCounterT event_counter_t;

	event_counter_t& getEventCounter() {
		return event_counter_;
	}

  protected:
	FunctionCache(user_func_t&& user_func) :
			user_func_(std::move(user_func)) {}

	~FunctionCache() = default;

	FunctionCache(const FunctionCache&) = delete;
	FunctionCache& operator =(const FunctionCache&) = delete;

	return_t invokeUserFunc(args_tuple_t&& args) {
		event_counter_.user_func_calls++;
		return std::experimental::apply(user_func_, std::move(args));
	}

  private:
	user_func_t user_func_;
	event_counter_t event_counter_;
};

#endif //NUMDB_NUMDB_H

