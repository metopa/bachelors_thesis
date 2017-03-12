/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#include <random>

#include <benchmark/benchmark.h>

#include "numdb/numdb.h"
#include "utils.h"


template <int N, typename Result, typename... Args>
struct Fibonacci {
	Result operator ()(Args...) {
		volatile int n = N;
		n = fibonacciImpl(n);
		return n;
	}
  private:
	int fibonacciImpl(int n) {
		return n <= 2 ? 1 : fibonacciImpl(n - 1) + fibonacciImpl(n - 2);
	}
};

void BM_DummyCache(benchmark::State& state) {
	DummyFunctionCache<
			Fibonacci<20, int, double, double>,
			std::tuple<double, double>,
			BasicEventCounter> cache((Fibonacci<20, int, double, double>()));

/*	std::random_device r;

	// Choose a random mean between 1 and 6
	std::mt19937 e2(r());
	std::normal_distribution<double> normal_dist
			(0, computeSigma(static_state.range(1), state.range(0)));*/

	while (state.KeepRunning()) {
		cache(1, 2);
	}
}


void BM_FibII(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(Fibonacci<20, int>()());
	}
}


// Register the function as a benchmark
BENCHMARK(BM_FibII);
BENCHMARK(BM_DummyCache);

BENCHMARK_MAIN();
