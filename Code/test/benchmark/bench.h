/** @file bench.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_BENCH_H
#define NUMDB_BENCH_H

#include <cstddef>

struct BenchProfile {
	int thread_count;
	size_t available_memory;
	double cache_size_to_distribution_area_ration;
};


#endif //NUMDB_BENCH_H
