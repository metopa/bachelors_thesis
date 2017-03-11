/** @file dummy_engine.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_DUMMY_ENGINE_H
#define NUMDB_DUMMY_ENGINE_H

#include <type_traits>
#include <tuple>

template<typename EventCounterT,
		typename UserFunc,
		typename... UserFuncArgs>
class DummyEngine {
  public:
	typedef UserFunc user_func_t;
	typedef std::result_of return_t;
	typedef std::tuple<UserFuncArgs> args_tuple_t;

	DummyEngine(size_t /*available_memory*/, user_func_t user_func) :
			user_func_(std::move(user_func)) {}

	return_t retrieve(UserFuncArgs&& ... args) {
		event_counter_.total_retrieves++;
		event_counter_.user_func_calls++;
		return user_func_(std::forward<UserFuncArgs>(args)...);
	}

	EventCounterT& getEventCounter() {
		return event_counter_;
	}

  private:
	user_func_t user_func_;
	EventCounterT event_counter_;
};


#endif //NUMDB_DUMMY_ENGINE_H
