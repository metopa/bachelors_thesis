/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>

#include "benchmark/benchmark.h"
#include "utils.h"

#include "numdb/numdb.h"

#ifndef SHORT_BENCHMARK
#define SHORT_BENCHMARK 1
#endif

using namespace numdb;
using containers::CanonicalSplayStrategy;
using containers::WstSplayStrategy;
using containers::AccessCountSplayStrategy;
using containers::ParametrizedAccessCountSplayStrategy;

//FIXME compute sigma parameter basing on the function arity
/**
 * @arg state.range(0) min argument for a Fibonacci function
 * @arg state.range(1) max argument for a Fibonacci function
 * @arg state.range(2) available memory (in KiB)
 * @arg state.range(3) desired area under curve (in percents)
 * @arg state.range(4) distribution slide speed * 100
 */
template <class ContainerTypeHolder>
void BM(benchmark::State& state) {
	using UserFunc = Fibonacci<int, double>;

	size_t mem = static_cast<size_t>(state.range(2)) * 1024;
	size_t max_element_capacity = mem / UserFunc::aggregatedArgSize();

	double area = state.range(3);
	area /= 100;

	double mean_delta = state.range(4) / 100.;
	double mean_offset = 0;

	double sigma = computeSigma(area, max_element_capacity);
	std::random_device r;
	std::mt19937 e(r());
	std::normal_distribution<double> dist(0, sigma);

	FunctionCache<UserFunc, ContainerTypeHolder, utility::BasicEventCounter>
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

constexpr int maxKB = 2 * 1024 * 1024;

void SeqArguments(benchmark::internal::Benchmark* b) {
	std::vector<int> areas = {30,
	#if !SHORT_BENCHMARK
							  75,
	#endif
							  95};
	int min_memory = 100;
	int max_memory = maxKB;
	std::vector<std::pair<int, int>> fib_args = {
	#if !SHORT_BENCHMARK
	                                             {30, 30},
												 {37, 37},
	#endif
												 {30, 37},
												};
	#if SHORT_BENCHMARK
	constexpr int mem_multiply = 16;
	#else
	constexpr int mem_multiply = 8;
	#endif
	std::vector<int> slide = {0, 10};
	for (auto slide_speed : slide)
		for (auto fib_arg : fib_args)
			for (auto area : areas)
				for (int mem = min_memory;; mem *= mem_multiply) {
					b->Args({fib_arg.first, fib_arg.second,
							 std::min(mem, max_memory), area, slide_speed});
					if (mem >= max_memory)
						break;
				}
}

#define BENCH_ARGS Apply(SeqArguments)

BENCHMARK_TEMPLATE(BM, DummyContainer)->BENCH_ARGS;

#if SHORT_BENCHMARK
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<0>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<1>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<2>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<4>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<16>)->BENCH_ARGS;
#else
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<1>)->BENCH_ARGS; //TODO Set optimal rate
#endif

#if SHORT_BENCHMARK
BENCHMARK_TEMPLATE(BM, PriorityHashtable<0>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, PriorityHashtable<1>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, PriorityHashtable<4>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, PriorityHashtable<16>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LruHashtable<>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LfuHashtable<>)->BENCH_ARGS;
#else
BENCHMARK_TEMPLATE(BM, PriorityHashtable<1>)->BENCH_ARGS; //TODO Set optimal rate
#endif

#if SHORT_BENCHMARK
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<CanonicalSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<AccessCountSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<WstSplayStrategy<1>>)->BENCH_ARGS;

BENCHMARK_TEMPLATE(BM, LruSplayTree<CanonicalSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LruSplayTree<AccessCountSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LruSplayTree<WstSplayStrategy<1>>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LruSplayTree<ParametrizedAccessCountSplayStrategy<2, 1, 8>>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LruSplayTree<ParametrizedAccessCountSplayStrategy<16, 1, 255>>)->BENCH_ARGS;

BENCHMARK_TEMPLATE(BM, LfuSplayTree<CanonicalSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LfuSplayTree<AccessCountSplayStrategy>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LfuSplayTree<WstSplayStrategy<1>>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LfuSplayTree<ParametrizedAccessCountSplayStrategy<2, 1, 8>>)->BENCH_ARGS;
BENCHMARK_TEMPLATE(BM, LfuSplayTree<ParametrizedAccessCountSplayStrategy<16, 1, 255>>)->BENCH_ARGS;
#else
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<CanonicalSplayStrategy>)->BENCH_ARGS;
#endif


BENCHMARK_MAIN();
