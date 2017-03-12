/** @file event_counter.h
 *  @brief This structure is used for measuring different metrics, like total count of retrieves, number of user function calls, etc.
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_EVENT_COUNTER_H
#define NUMDB_EVENT_COUNTER_H

#include <cstdint>
#include <ostream> //TODO Replace with <iosfwd>

struct EmptyEventCounter {
	void retrieve() {}
	void invokeUserFunc() {}
};

struct BasicEventCounter {
	uint64_t total_retrieves = 0;
	uint64_t user_func_invocations = 0;

	void retrieve() {
		total_retrieves++;
	}

	void invokeUserFunc() {
		user_func_invocations++;
	}

	double cacheEfficiency() const {
		return 1 - static_cast<double>(user_func_invocations) / total_retrieves;
	}

	friend std::ostream& operator <<(std::ostream& out, const BasicEventCounter& ec) {
		return out << "Total: " << ec.total_retrieves << std::endl
				   << "User func calls: " << ec.user_func_invocations << std::endl
				   << "Cache efficiency: " << ec.cacheEfficiency() << std::endl;
	}
};

#endif //NUMDB_EVENT_COUNTER_H
