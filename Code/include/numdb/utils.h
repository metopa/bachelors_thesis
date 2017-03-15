/** @file utils.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_BENCHMARK_UTILS_H
#define NUMDB_BENCHMARK_UTILS_H


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

#endif //NUMDB_BENCHMARK_UTILS_H
