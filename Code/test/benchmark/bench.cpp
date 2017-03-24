/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>
#include <numdb/function_cache.h>
#include <numdb/hash_table/fixed_hashtable_fair_lru.h>

#include "benchmark/benchmark.h"

#include "numdb/numdb.h"
#include "utils.h"

double computeScore(size_t total_retrievals,
					size_t user_func_calls, double area_under_curve) {

	return (total_retrievals - user_func_calls) / (double) total_retrievals * 100
		   / area_under_curve;
}

/**
 * @arg state.range(0) argument for a Fibonacci function
 * @arg state.range(1) available memory (in KiB)
 * @arg state.range(2) desired area under curve (in percents)
 */
void BM_Dummy(benchmark::State& state) {
	using UserFunc = Fibonacci<int, double, double>;
	size_t mem = static_cast<size_t>(state.range(1));
	mem *= 1024;
	size_t max_element_capacity = mem / UserFunc::aggregatedArgSize();
	double area = state.range(2);
	area /= 100;

	std::random_device r;
	std::mt19937 e(r());

	double sigma = computeSigma(area, max_element_capacity);
	std::normal_distribution<double> dist(0, sigma);

	DummyFunctionCache<UserFunc, BasicEventCounter>
			cache(UserFunc(state.range(0)), mem);

	while (state.KeepRunning()) {
		double a = dist(e);
		double b = dist(e);
		benchmark::DoNotOptimize(cache(a, b));
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["capacity"] = max_element_capacity;
	state.counters["score"] = computeScore(cache.eventCounter().total_retrievals,
										   cache.eventCounter().user_func_invocations, area);
}

/**
 * @arg state.range(0) argument for a Fibonacci function
 * @arg state.range(1) available memory (in KiB)
 * @arg state.range(2) desired area under curve (in percents)
 */
void BM_Hashtable(benchmark::State& state) {
	using UserFunc = Fibonacci<int, double>;

	size_t mem = static_cast<size_t>(state.range(1)) * 1024;
	size_t max_element_capacity = mem / UserFunc::aggregatedArgSize();

	double area = state.range(2);
	area /= 100;

	double sigma = computeSigma(area, max_element_capacity);
	std::random_device r;
	std::mt19937 e(r());
	std::normal_distribution<double> dist(0, sigma);

	FunctionCache<UserFunc, FixedHashtableFairLRUTypeHolder<>, BasicEventCounter>
			cache(UserFunc(state.range(0)), mem);

	while (state.KeepRunning()) {
		double a = std::round(dist(e));
		benchmark::DoNotOptimize(cache(a));
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["capacity"] = cache.capacity();
	state.counters["score"] = computeScore(cache.eventCounter().total_retrievals,
										   cache.eventCounter().user_func_invocations, area);
}

BENCHMARK(BM_Hashtable)->Args({25, 100, 30})->Args({25, 100, 50})->Args({25, 100, 70})->Args({25, 100, 90});
BENCHMARK(BM_Dummy)->Args({25, 100, 30});

BENCHMARK_MAIN();
