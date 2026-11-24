#ifndef SANDBOX_MODULE_MAP_HPP_
#define SANDBOX_MODULE_MAP_HPP_

#include <array>
#include <cstdint>
#include <optional>

#include "module_position.hpp"
#include "module_types.hpp"

namespace sndbx {

template <typename TypeErasedWrapper, std::size_t N> class ModuleMap {
  public:
	template <class Module> void add(Module *module, ModulePosition pos) {
		assert(!is_full());
		auto &entry = data_[pos.value];
		assert(!entry.has_value());
		entry = {TypeErasedWrapper{module}, module_type<Module>,
		         (void *)module};
		++size_;
	}

	// ReleaseFunc must take (ModuleType, void *) as args
	template <typename ReleaseFunc>
	void remove(ModulePosition pos, ReleaseFunc &&func) {
		auto &entry = get(pos);
		func(entry->type, entry->ptr);
		entry.reset();
		--size_;
	}

	[[nodiscard]] bool is_occupied(ModulePosition pos) const {
		return data_[pos.value].has_value();
	}

	[[nodiscard]] const auto &operator[](ModulePosition pos) const {
		const auto &entry = get(pos);
		return entry->module;
	}

	[[nodiscard]] auto &operator[](ModulePosition pos) {
		auto &entry = get(pos);
		return entry->module;
	}

	[[nodiscard]] auto get_module_type(ModulePosition pos) const {
		const auto &entry = get(pos);
		return entry->type;
	}

	[[nodiscard]] const auto size() const { return size_; }

	[[nodiscard]] bool is_full() const { return size_ == N; }

	const auto &data() const { return data_; }
	auto &data() const { return data_; }

  private:
	auto &get(ModulePosition pos) {
		auto &entry = data_[pos.value];
		assert(entry.has_value());
		return entry;
	}

	const auto &get(ModulePosition pos) const {
		const auto &entry = data_[pos.value];
		assert(entry.has_value());
		return entry;
	}

	struct Entry {
		TypeErasedWrapper module;
		ModuleType type;
		void *ptr;
	};
	std::array<std::optional<Entry>, N> data_{};
	std::size_t size_;
};

} // namespace sndbx

#endif