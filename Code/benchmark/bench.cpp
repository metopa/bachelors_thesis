/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>
#include <memory>
#include <benchmark/benchmark.h>
#include "utils.h"

#include "numdb/numdb.h"

#define PARALLEL_ONLY 0

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

/**
 * @arg state.range(0) min argument for a Fibonacci function
 * @arg state.range(1) max argument for a Fibonacci function
 * @arg state.range(2) available memory (in KiB)
 * @arg state.range(3) desired area under curve (in percents)
 * @arg state.range(4) distribution slide speed * 100
 */
template <class ContainerTypeHolder, size_t... ContainerArgs>
void ParallelBM(benchmark::State& state) {
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


	using cache_t = FunctionCache<UserFunc, ContainerTypeHolder, utility::BasicEventCounter>;
	static std::unique_ptr<cache_t> cache_ptr;

	if (state.thread_index == 0)
		cache_ptr.reset(new cache_t(UserFunc(state.range(0), state.range(1)), mem, ContainerArgs...));

	while (state.KeepRunning()) {
		double a = std::round(dist(e) + mean_offset);
		benchmark::DoNotOptimize((*cache_ptr)(a));
		mean_offset += mean_delta;
	}

	state.SetItemsProcessed(state.iterations());

	if (state.thread_index == 0) {
		state.counters["cap"] = cache_ptr->capacity();
		state.counters["hr"] = computeHitRate(cache_ptr->eventCounter().total_retrievals,
											  cache_ptr->eventCounter().user_func_invocations, area);
		cache_ptr.reset();
	}
}

constexpr int maxKB = 100 * 1024;
constexpr int maxThreads = 2;

void shortSequence(benchmark::internal::Benchmark* b) {
	constexpr int min_memory = 1;
	constexpr int max_memory = PARALLEL_ONLY ? 256 : 256;
	constexpr int mem_multiply = 4;

	std::vector<int> areas = {85};
	std::vector<std::pair<int, int>> fib_args = {{30, 35}};
	std::vector<int> slide = {1};

	b->ArgNames({"fib_min", "fib_max", "memory", "area", "mean_varying_rate"});

	for (auto slide_speed : slide)
		for (auto fib_arg : fib_args)
			for (auto area : areas)
				for (int mem = min_memory; mem <= max_memory; mem *= mem_multiply) {
					b->Iterations(mem * 250 * 100 / area + 5000)->
							Args({fib_arg.first, fib_arg.second,
								  std::min(mem, max_memory), area, slide_speed});
				}
}

void longSequence(benchmark::internal::Benchmark* b) {
	int min_memory = 1;
	int max_memory = maxKB;
	constexpr int mem_multiply = 4;

	std::vector<int> areas = {60, 95};
	std::vector<std::pair<int, int>> fib_args = {{28, 35}};
	std::vector<int> slide = {0, 10};

	b->ArgNames({"fib_min", "fib_max", "memory", "area", "mean_varying_rate"});

	for (auto& slide_speed : slide)
		for (auto& fib_arg : fib_args)
			for (auto& area : areas)
				for (int mem = min_memory;; mem *= mem_multiply) {
					b->Iterations(mem * 1024 * 10 * 100 / area)->Args({fib_arg.first, fib_arg.second,
																	   std::min(mem, max_memory), area, slide_speed});
					if (mem >= max_memory)
						break;
				}
}

BENCHMARK_TEMPLATE(BM, DummyContainer)->Iterations(2000)->Args({30, 35, 100, 50, 0});

#if !PARALLEL_ONLY
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<0>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<1>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<2>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<4>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, WeightedSearchTree<16>)->Apply(shortSequence);


BENCHMARK_TEMPLATE(BM, PriorityHashtable<0>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, PriorityHashtable<1>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, PriorityHashtable<2>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, PriorityHashtable<4>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, PriorityHashtable<16>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LruHashtable<>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LfuHashtable<>)->Apply(shortSequence);

BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<CanonicalSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<AccessCountSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, BottomNodeSplayTree<WstSplayStrategy<1>>)->Apply(shortSequence);

BENCHMARK_TEMPLATE(BM, LruSplayTree<CanonicalSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LruSplayTree<AccessCountSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LruSplayTree<WstSplayStrategy<1>>)->Apply(shortSequence);

BENCHMARK_TEMPLATE(BM, LfuSplayTree<CanonicalSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LfuSplayTree<AccessCountSplayStrategy>)->Apply(shortSequence);
BENCHMARK_TEMPLATE(BM, LfuSplayTree<WstSplayStrategy<1>>)->Apply(shortSequence);

#endif

BENCHMARK_TEMPLATE(ParallelBM, CoarseLockAdapter<WeightedSearchTree<0>>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);
BENCHMARK_TEMPLATE(ParallelBM, CoarseLockAdapter<BottomNodeSplayTree<CanonicalSplayStrategy>>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);

BENCHMARK_TEMPLATE(ParallelBM, BinningConcurrentAdapter<WeightedSearchTree<0>, maxThreads>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);
BENCHMARK_TEMPLATE(ParallelBM, BinningConcurrentAdapter<BottomNodeSplayTree<CanonicalSplayStrategy>, maxThreads>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);


BENCHMARK_TEMPLATE(ParallelBM, CNDC<false>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);
BENCHMARK_TEMPLATE(ParallelBM, CNDC<true>)
->ThreadRange(1, maxThreads)->Apply(shortSequence);

BENCHMARK_MAIN();
