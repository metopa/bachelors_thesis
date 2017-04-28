/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>
#include <numdb/splay_tree/splay_tree_strategy.h>
#include <numdb/wst/weighted_search_tree.h>
#include <numdb/fair_lfu.h>
#include <numdb/hash_table/fixed_hashtable_binary_heap.h>

#include "benchmark/benchmark.h"

#include "numdb/numdb.h"
#include "utils.h"

double computeHitRate(size_t total_retrievals,
					  size_t user_func_calls,
					  double area_under_curve) {
	return (total_retrievals - user_func_calls) /
		   (double) total_retrievals * 100;
}

//FIXME compute sigma parameter basing on the function arity
/**
 * @arg state.range(0) min argument for a Fibonacci function
 * @arg state.range(1) max argument for a Fibonacci function
 * @arg state.range(2) available memory (in KiB)
 * @arg state.range(3) desired area under curve (in percents)
 * @arg state.range(4) distribution slide speed^-1
 */
template <class ContainerTypeHolder>
void BM(benchmark::State& state) {
	using UserFunc = Fibonacci<int, double>;

	size_t mem = static_cast<size_t>(state.range(2)) * 1024;
	size_t max_element_capacity = mem / UserFunc::aggregatedArgSize();

	double area = state.range(3);
	area /= 100;

	double mean_delta = state.range(4) == 0 ? 0 : 1. / state.range(4);
	double mean_offset = 0;

	double sigma = computeSigma(area, max_element_capacity);
	std::random_device r;
	std::mt19937 e(r());
	std::normal_distribution<double> dist(0, sigma);

	FunctionCache<UserFunc, ContainerTypeHolder, BasicEventCounter>
			cache(UserFunc(state.range(0), state.range(1)), mem);

	while (state.KeepRunning()) {
		double a = std::round(dist(e) + mean_offset);
		benchmark::DoNotOptimize(cache(a));
		mean_offset += mean_delta;
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["cap"] = cache.capacity();
	state.counters["hr"] = computeHitRate(cache.eventCounter().total_retrievals,
										  cache.eventCounter().user_func_invocations, area);
}

constexpr int max_arg = 33;
constexpr int min_arg = 29;

#define BENCH_ARGS Args({min_arg, max_arg, 10, 90, 0})->Args({min_arg, max_arg, 10, 90, 10})//->Args({min_arg, max_arg, 1000, 95, 1})->Args({min_arg, max_arg, 1000, 55, 1})


BENCHMARK_TEMPLATE(BM, FixedHashtableBinaryHeapTypeHolder<0>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, FixedHashtableBinaryHeapTypeHolder<1>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, FixedHashtableBinaryHeapTypeHolder<16>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeBottomNodeTypeHolder<CanonicalSplayStrategy>)->BENCH_ARGS;

BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<0>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<1>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<2>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, FixedHashtableFairLeastUsedTypeHolder<FairLRU>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, FixedHashtableFairLeastUsedTypeHolder<FairLFU>)->BENCH_ARGS;

//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<FairLFU, CanonicalSplayStrategy>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<FairLRU, CanonicalSplayStrategy>)->BENCH_ARGS;


//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<FairLRU, AccessCountSplayStrategy>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, SplayTreeBottomNodeTypeHolder<AccessCountSplayStrategy>)->BENCH_ARGS;

//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<ParametrizedAccessCountSplayStrategy<2, 1, 8>>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<AccessCountSplayStrategy>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, SplayTreeFairLeastUsedTypeHolder<WstSplayStrategy<1>>)->BENCH_ARGS;


BENCHMARK_TEMPLATE(BM, DummyContainerTypeHolder)->BENCH_ARGS;

BENCHMARK_MAIN();
