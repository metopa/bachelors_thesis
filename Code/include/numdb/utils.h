/** @file utils.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_BENCHMARK_UTILS_H
#define NUMDB_BENCHMARK_UTILS_H

struct Empty{};

template <class T>
struct CacheContainerTraits {
	static_assert(sizeof(T) < 0, "No traits specialization for this type");
};

template <class F>
decltype(auto) defer_call(F&& f) {
	struct foo {
		F f;
		~foo() { f(); };
	};
	return foo{std::move(f)};
}

#define TOKENPASTE_HELPER(x, y) x ## y
#define TOKENPASTE2_HELPER(x, y) TOKENPASTE_HELPER(x, y)
#define DEFERRED(FUNC) auto TOKENPASTE2_HELPER(_deferred_, __LINE__) = defer_call([&]() {FUNC;});


#endif //NUMDB_BENCHMARK_UTILS_H
