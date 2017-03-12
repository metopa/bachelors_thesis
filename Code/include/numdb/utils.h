/** @file utils.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_UTILS_H
#define NUMDB_UTILS_H

#include <tuple>

template <typename ArgsTupleT, typename... Args>
void checkArgsCanBeConvertedIntoTuple() {
	static_assert(std::is_convertible<
						  std::tuple<Args...>,
						  ArgsTupleT
				  >::value,
				  "Cannot convert provided arguments.");
};

#endif //NUMDB_UTILS_H
