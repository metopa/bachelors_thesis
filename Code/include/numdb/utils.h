/** @file utils.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_BENCHMARK_UTILS_H
#define NUMDB_BENCHMARK_UTILS_H

struct Empty{};

template <class T>
struct CacheContainerTraits {
	static_assert(sizeof(T) < 0, "No traits specialization for this type");
};

template <std::size_t>
struct int_ {};

template <class Tuple, size_t Pos>
std::ostream& print_tuple(std::ostream& out, const Tuple& t, int_<Pos>) {
	out << std::get<std::tuple_size<Tuple>::value - Pos>(t) << ',';
	return print_tuple(out, t, int_<Pos - 1>());
}

template <class Tuple>
std::ostream& print_tuple(std::ostream& out, const Tuple& t, int_<1>) {
	return out << std::get<std::tuple_size<Tuple>::value - 1>(t);
}

template <class... Args>
std::ostream& operator <<(std::ostream& out, const std::tuple<Args...>& t) {
	out << '(';
	print_tuple(out, t, int_<sizeof...(Args)>());
	return out << ')';
}

template <class F>
decltype(auto) defer_call(F&& f) {
	struct foo {
		F f;
		~foo() { f(); };
	};
	return foo{std::move(f)};
}

#define TOKENPASTE_HELPER(x, y) x ## y
#define TOKENPASTE2_HELPER(x, y) TOKENPASTE_HELPER(x, y)
#define DEFERRED(FUNC) auto TOKENPASTE2_HELPER(_deferred_, __LINE__) = defer_call([&]() {FUNC;});

template <typename T>
class FlaggedPointer {
	union {
		T* ptr_;
		uintptr_t val_;
	};
  public:
	FlaggedPointer(T* ptr = nullptr, bool flag = false) {
		ptr_ = ptr;
		setFlag(flag);
	}

	FlaggedPointer& operator=(T* ptr) {
		bool current_flag = flag();
		ptr_ = ptr;
		setFlag(current_flag);
		return *this;
	}

	void setFlag(bool flag) {
		val_ &= ~(uintptr_t) 1;
		val_ |= flag;
	}
	bool flag() const {
		return (bool) (val_ & 1);
	}

	T& operator *() {
		return ptr_;
	}
	const T& operator *() const {
		return ptr_;
	}
	T* operator ->() {
		return ptr_;
	}
	T* operator ->() const {
		return ptr_;
	}

};

#endif //NUMDB_BENCHMARK_UTILS_H
