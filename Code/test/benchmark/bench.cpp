/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>

#include <benchmark/benchmark.h>

#include "numdb/numdb.h"
#include "utils.h"

/**
 * @arg state.range(0) argument for a Fibonacci function
 * @arg state.range(1) available memory (in KiB)
 * @arg state.range(2) desired area under curve (in percents)
 */
void BM_DummyCache(benchmark::State& state) {
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
		volatile double a = dist(e);
		volatile double b = dist(e);
		benchmark::DoNotOptimize(cache(a, b));
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["capacity"] = max_element_capacity;
	state.counters["overhead"] = 0;
	state.counters["direct calls [%]"] =
			cache.eventCounter().user_func_invocations / (double) state.iterations() * 100;
}


void BM_Fib(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(Fibonacci<int>(20)());
	}
	state.SetItemsProcessed(state.iterations());
}


// Register the function as a benchmark
BENCHMARK(BM_Fib);
BENCHMARK(BM_DummyCache)->Args({20, 1, 30})->Args({25, 2, 30});

BENCHMARK_MAIN();
