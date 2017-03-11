/** util.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_UTIL_H
#define NUMDB_UTIL_H

#include <boost/math/distributions/normal.hpp>

/**
 * @brief Compute sigma of a normal distribution (mean = 0) so that the range [-element_count / 2, element_count / 2] takes 'area' of area under the pdf curve
 * @param area
 * @param element_count
 * @return
 */
double computeSigma(double area, size_t element_count) {
	boost::math::normal N01;  // 'Standard' normal distribution with zero mean
	double q = boost::math::quantile(N01, 0.5 + area / 2);
	return element_count / 2 / q;
}

#endif //NUMDB_UTIL_H
