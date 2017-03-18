/** @file utils.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_BENCHMARK_UTILS_H
#define NUMDB_BENCHMARK_UTILS_H

struct Empty{};

template <class T>
struct HashtableTraits {
	static_assert(sizeof(T) < 0, "No traits specialization for this type");
};

#endif //NUMDB_BENCHMARK_UTILS_H
