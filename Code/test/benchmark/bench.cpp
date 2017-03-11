/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#include "bench.h"
#include <benchmark/benchmark.h>
#include <numdb/numdb.h>


template<int N, typename Result>
struct Fibonacci {
	template<typename... Args>
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

template <class Engine, typename... Args>
void BM_Sequential(benchmark::State& state) {
	FunctionCache<Engine, Args...> cache(state.range(0) * );
	while (state.KeepRunning())
}


void BM_FibII(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(fibonacci<20, int>(int()));
	}
}

void BM_FibIIDD(benchmark::State& state) {
	while (state.KeepRunning()) {
		benchmark::DoNotOptimize(fibonacci<20, int>(int(), double(), double()));
	}
}

// Register the function as a benchmark
BENCHMARK(BM_FibII);
BENCHMARK(BM_FibIIDD);

BENCHMARK_MAIN();
