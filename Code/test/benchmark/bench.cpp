/** @file bench.cpp
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <random>
#include <numdb/splay_tree/splay_tree_strategy.h>


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
 * @arg state.range(0) argument for a Fibonacci function
 * @arg state.range(1) available memory (in KiB)
 * @arg state.range(2) desired area under curve (in percents)
 */
template <class ContainerTypeHolder>
void BM(benchmark::State& state) {
	using UserFunc = Fibonacci<int, double>;

	size_t mem = static_cast<size_t>(state.range(1)) * 1024;
	size_t max_element_capacity = mem / UserFunc::aggregatedArgSize();

	double area = state.range(2);
	area /= 100;

	double sigma = computeSigma(area, max_element_capacity);
	std::random_device r;
	std::mt19937 e(r());
	std::normal_distribution<double> dist(0, sigma);

	FunctionCache<UserFunc, ContainerTypeHolder, BasicEventCounter>
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

BENCHMARK_TEMPLATE(BM, FixedHashtableFairLRUTypeHolder<>)->Args({27, 100, 70});
BENCHMARK_TEMPLATE(BM, SplayTreeFairLRUTypeHolder<CanonicalSplayStrategy>)->
		Args({27, 100, 70});
BENCHMARK_TEMPLATE(BM, DummyContainerTypeHolder)->Args({27, 100, 70});

BENCHMARK_MAIN();
