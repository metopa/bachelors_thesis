/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>

#include "benchmark/benchmark.h"
#include "numdb/fixed_hashtable_function_cache.h"

#include "numdb/numdb.h"
#include "utils.h"

double computeHitRate(size_t total_retrievals,
					  size_t user_func_calls) {

	return (total_retrievals - user_func_calls) / (double) total_retrievals * 100;
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
	state.counters["hitrate"] = computeHitRate(cache.eventCounter().total_retrievals,
											   cache.eventCounter().user_func_invocations);
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

	FixedHashtableFunctionCache<UserFunc, BasicEventCounter>
			cache(UserFunc(state.range(0)), mem);

	while (state.KeepRunning()) {
		double a = std::round(dist(e));
		benchmark::DoNotOptimize(cache(a));
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["capacity"] = cache.capacity();
	state.counters["hitrate"] = computeHitRate(cache.eventCounter().total_retrievals,
											   cache.eventCounter().user_func_invocations);
}


void BM_Fib(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(Fibonacci<int>(20)());
	}
	state.SetItemsProcessed(state.iterations());
}


// Register the function as a benchmark
BENCHMARK(BM_Hashtable)->Args({25, 100, 30})->Args({25, 100, 50})->Args({25, 100, 70});
BENCHMARK(BM_Dummy)->Args({25, 100, 30});

BENCHMARK_MAIN();
