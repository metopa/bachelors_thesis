/** @file initial_priority_generator.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#include <cstdint>
#include <algorithm>

#ifndef NUMDB_INITIAL_PRIORITY_GENERATOR_H
#define NUMDB_INITIAL_PRIORITY_GENERATOR_H

template <
		uint64_t MaxPriority,
		uint64_t InitialLearningIterations = 100,
		uint64_t IterationsUntilUpdate = 1024>
class InitialPriorityGenerator {
	uint64_t sum_ = 0;
	uint64_t count_ = 0;
	uint64_t min_ = static_cast<uint64_t>(-1);
	uint64_t max_ = 0;

  public:
	uint64_t calculatePriority(uint64_t duration) {
		sum_ += duration;
		count_++;

		if (count_ > IterationsUntilUpdate) {
			count_ /= 2;
			sum_ /= 2;
			uint64_t avg = sum_ / count_;
			uint64_t delta = std::max(avg - min_,
									  max_ - avg);
			min_ = avg - delta;
			max_ = avg + delta;
		}

		if (duration < min_)
			min_ = duration;
		else if (duration > max_)
			max_ = duration;

		if (count_ < InitialLearningIterations)
			return MaxPriority / 2;
		else
			return max_ == min_ ?
				   MaxPriority / 2 :
				   std::min(MaxPriority * (duration - min_) /
							(max_ - min_), MaxPriority - 1);
	}
};

#endif //NUMDB_INITIAL_PRIORITY_GENERATOR_H
