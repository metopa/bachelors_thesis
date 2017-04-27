/** @file binning_concurrent_adapter.h
 *  @brief
 *
 *  @author Viacheslav Kroilov (metopa) <slavakroilov@gmail.com>
 */

#ifndef NUMDB_BINNING_CONCURRENT_ADAPTER_H
#define NUMDB_BINNING_CONCURRENT_ADAPTER_H

#include <cstddef>
#include <experimental/optional>
#include <mutex>
#include <vector>
#include <random>

#include <murmurhash2/all.h>

template <typename KeyT, typename ValueT, typename ContainerTypeHolderT>
class BinningConcurrentAdapter;

template <typename ContainerTypeHolderT>
struct BinningConcurrentAdapterTypeHolder {
	template <typename KeyT, typename ValueT>
	using container_t = BinningConcurrentAdapter<KeyT, ValueT, ContainerTypeHolderT>;
};

template <typename KeyT, typename ValueT, typename ContainerTypeHolderT>
class BinningConcurrentAdapter {
  public:
	using inner_container_t = typename ContainerTypeHolderT::template container_t<KeyT, ValueT>;

	template <typename... Args>
	BinningConcurrentAdapter(size_t available_memory,
							size_t bins_count,
							Args... container_args) {
		if (bins_count == 0)
			throw std::invalid_argument("Binning container adapter: invalid bin count");
		hash_seed_ = std::random_device()();
		bins_.reserve(bins_count);
		for (size_t i = 0; i < bins_count; i++)
			bins_.emplace_back(available_memory / bins_count, container_args...);
		mutexes_.resize(bins_count);
	}

	size_t capacity() const {
		return bins_[0].capacity() * bins_.size();
	}

	size_t size() const {
		size_t total = 0;
		for (auto& b : bins_)
			total += b.size();
		return total;
	}

	static constexpr bool isThreadsafe() {
		return true;
	}

	static constexpr size_t elementSize() {
		return inner_container_t::elementSize();
	}

	std::experimental::optional<ValueT> find(const KeyT& key) {
		size_t bin = getBin(key);
		std::lock_guard(mutexes_[bin]);
		return bins_[bin].find(key);
	}

	void insert(const KeyT& key, const ValueT& value, size_t priority) {
		size_t bin = getBin(key);
		std::lock_guard(mutexes_[bin]);
		return bins_[bin].insert(key, value, priority);
	}

  private:
	size_t getBin(const KeyT& key) const {
		return mmh2::getMurmurHash2(key, hash_seed_) % bins_.size();
	}

	size_t hash_seed_;

	std::vector<inner_container_t> bins_;
	std::vector<std::mutex> mutexes_;
};

#endif //NUMDB_BINNING_CONCURRENT_ADAPTER_H
