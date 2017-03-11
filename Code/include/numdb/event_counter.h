/** @file event_counter.h
 *  @brief This structure is used for measuring different metrics, like total count of retrieves, number of user function calls, etc.
 *
 *  @author Viacheslav Kroilov (metopa) slavakroilov@gmail.com
 */

#ifndef NUMDB_EVENT_COUNTER_H
#define NUMDB_EVENT_COUNTER_H

#include <cstdint>
#include <ostream> //TODO Replace with <iosfwd>

struct EventCounter {
	uint64_t total_retrieves = 0;
	uint64_t user_func_calls = 0;

	double cacheEfficiency() const {
		return 1 - static_cast<double>(user_func_calls) / total_retrieves;
	}

	friend std::ostream& operator<<(std::ostream& out, const EventCounter& ec) {
		return out << "Total: " << ec.total_retrieves << std::endl
				   << "User func calls: " << ec.user_func_calls << std::endl
				   << "Cache efficiency: " << ec.cacheEfficiency() << std::endl;
	}
};

#endif //NUMDB_EVENT_COUNTER_H
