/** @file util.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
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


template <typename Result, typename... Args>
struct Fibonacci {
	Fibonacci(int fib_arg) : fib_arg_(fib_arg) {}

	Result operator ()(Args...) {
		volatile int n = fib_arg_;
		n = fibonacciImpl(n);
		return n;
	}

	static size_t aggregatedArgSize() {
		return sizeof(std::tuple<Result, Args...>);
	}
  private:
	int fib_arg_;
	int fibonacciImpl(int n) {
		return n <= 2 ? 1 : fibonacciImpl(n - 1) + fibonacciImpl(n - 2);
	}
};

#endif //NUMDB_UTIL_H
