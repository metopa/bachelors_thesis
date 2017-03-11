/** @file numdb.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_NUMDB_H
#define NUMDB_NUMDB_H


#include <type_traits>
#include <tuple>

template<typename Engine, typename UserFunc, typename... UserFuncArgs>
class FunctionCache {
  public:
	typedef UserFunc user_func_t;
	typedef std::result_of return_t;
	typedef std::tuple<UserFuncArgs> args_tuple_t;

	template<typename... EngineCtorArgs>
	FunctionCache(EngineCtorArgs&&... engine_args) :
			engine_(std::forward<EngineCtorArgs>(engine_args)...) {}

	return_t retrieve(UserFuncArgs&&... args) {
		return engine_.retrieve(std::forward<UserFuncArgs>(args)...);
	}

  private:
	Engine engine_;
};

#endif //NUMDB_NUMDB_H
