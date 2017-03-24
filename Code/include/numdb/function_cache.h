/** @file function_cache.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_FUNCTION_CACHE_H
#define NUMDB_FUNCTION_CACHE_H

#include <murmurhash2/all.h>
#include <function_traits/functional_unwrap.hpp>
#include "numdb/function_cache_core.h"
#include "numdb/event_counter.h"
#include "numdb/utils.h"

template <
		typename UserFuncT,
		typename ContainerTypeHolder,
		typename EventCounterT = EmptyEventCounter,
		typename UserFuncArgsTupleT = fu::argument_tuple_type_of_t<UserFuncT>
>
class FunctionCache {
	using core_t = FunctionCacheCore
			<UserFuncT, UserFuncArgsTupleT, EventCounterT>;
	using container_t = typename ContainerTypeHolder::template container_t<
			typename core_t::args_tuple_t, typename core_t::return_t>;

  public:
	template <typename... ContainerArgs>
	FunctionCache(UserFuncT user_func, size_t available_memory,
				  ContainerArgs&& ... container_args) :
			core_(std::move(user_func)),
			container_(available_memory,
					   std::forward<ContainerArgs>(container_args)...) {}

	size_t capacity() const {
		return container_.capacity();
	}

	size_t size() const {
		return container_.size();
	}

	static constexpr bool isThreadsafe() {
		return container_t::isThreadsafe();
	}

	size_t elementSize() const {
		return container_.elementSize();
	}

	double elementSizeOverhead() const {
		return 1 - static_cast<double>(sizeof(typename core_t::args_tuple_t) +
									   sizeof(typename core_t::return_t))
				   / elementSize();
	}

	EventCounterT& eventCounter() {
		return core_.getEventCounter();
	}

	template <typename... Args>
	auto operator ()(Args&& ... args) {
		static_assert(
				std::is_convertible<
						std::tuple<Args...>,
						typename core_t::args_tuple_t>::value,
				"Cannot convert provided arguments.");

		core_.getEventCounter().retrieve();

		typename core_t::args_tuple_t key(std::forward<Args>(args)...);

		auto res = container_.find(key);
		if (res) {
			return *res;
		}

		auto computed_res = core_.invokeUserFunc(key);
		container_.insert(std::move(key), computed_res);
		return std::move(computed_res);
	}

  private:
	core_t core_;
	container_t container_;
};

#endif //NUMDB_FUNCTION_CACHE_H
