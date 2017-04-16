/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>
#include <numdb/splay_tree/splay_tree_strategy.h>
#include <numdb/wst/weighted_search_tree.h>


#include "benchmark/benchmark.h"

#include "numdb/numdb.h"
#include "utils.h"

double computeScore(size_t total_retrievals,
					size_t user_func_calls, double area_under_curve) {

	return (total_retrievals - user_func_calls) / (double) total_retrievals * 100
		   / area_under_curve;
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

	double mean_speed = state.range(4) == 0 ? 0 : 1. / state.range(4);
	double mean_delta = 0;

	double sigma = computeSigma(area, max_element_capacity);
	std::random_device r;
	std::mt19937 e(r());
	std::normal_distribution<double> dist(0, sigma);

	FunctionCache<UserFunc, ContainerTypeHolder, BasicEventCounter>
			cache(UserFunc(state.range(0), state.range(1)), mem);

	while (state.KeepRunning()) {
		double a = std::round(dist(e) + mean_delta);
		benchmark::DoNotOptimize(cache(a));
		mean_delta += mean_speed;
	}

	state.SetItemsProcessed(state.iterations());
	state.counters["capacity"] = cache.capacity();
	state.counters["score"] = computeScore(cache.eventCounter().total_retrievals,
										   cache.eventCounter().user_func_invocations, area);
}
constexpr int max_arg = 29;
constexpr int min_arg = 33;

#define BENCH_ARGS Args({min_arg, max_arg, 10, 70, 10})

BENCHMARK_TEMPLATE(BM, FixedHashtableFairLRUTypeHolder<>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeBottomNodeTypeHolder<CanonicalSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeBottomNodeTypeHolder<AccessCountSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeFairLRUTypeHolder<CanonicalSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeFairLRUTypeHolder<ParametrizedAccessCountSplayStrategy<2, 1, 8>>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, SplayTreeFairLRUTypeHolder<AccessCountSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<1>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<2>)->BENCH_ARGS;
//BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<3>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTreeTypeHolder<64>)->BENCH_ARGS;

BENCHMARK_TEMPLATE(BM, DummyContainerTypeHolder)->
		Args({min_arg, max_arg, 10, 70});

BENCHMARK_MAIN();
