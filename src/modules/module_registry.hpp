#ifndef SANDBOX_MODULE_REGISTRY_HPP_
#define SANDBOX_MODULE_REGISTRY_HPP_

#include <algorithm>
#include <cstddef>
#include <optional>

#include "nst/inplace_vector.hpp"
#include "nst/poly_view.hpp"
#include "nst/strong_alias.hpp"

#include "module_id.hpp"
#include "module_position.hpp"
#include "module_types.hpp"

namespace sndbx {

template <std::size_t Capacity, class... ModuleBases> class ModuleRegistry {
  public:
	using value_type = nst::poly_view<ModuleBases...>;

	template <class Module>
	[[nodiscard]] auto add(Module *m) -> std::optional<ModuleID> {
		if (modules_.is_full()) {
			return std::nullopt;
		}
		const ModuleID new_id{reinterpret_cast<std::uint32_t>(m)};
		ids_.push_back(new_id);
		modules_.emplace_back(m);
		return new_id;
	}

	[[nodiscard]] const auto &operator[](ModuleID id) const {
		return modules_[index_of(id)];
	}
	[[nodiscard]] auto &operator[](ModuleID id) {
		return modules_[index_of(id)];
	}

	bool erase(ModuleID id) {
		const auto index = index_of(id);
		if (index == ids_.size()) {
			return false;
		}
		modules_.erase(modules_.begin() + index);
		ids_.erase(ids_.begin() + index);
		return true;
	}

	[[nodiscard]] auto find(ModuleID id) const {
		const auto index = index_of(id);
		return (index == ids_.size()) ? cend() : index + cbegin();
	}

	[[nodiscard]] auto find(ModuleID id) {
		const auto index = index_of(id);
		return (index == ids_.size()) ? end() : index + begin();
	}

	void clear() {
		modules_.clear();
		ids_.clear();
	}

	[[nodiscard]] auto begin() { return modules_.begin(); }
	[[nodiscard]] auto end() { return modules_.end(); }
	[[nodiscard]] auto begin() const { return modules_.cbegin(); }
	[[nodiscard]] auto end() const { return modules_.cend(); }
	[[nodiscard]] auto cbegin() const { return begin(); }
	[[nodiscard]] auto cend() const { return end(); }

	[[nodiscard]] const auto &ids() const { return ids_; }

	[[nodiscard]] auto size() const { return modules_.size(); }

  private:
	std::size_t index_of(ModuleID id) const {
		const auto it = std::find(ids_.begin(), ids_.end(), id);
		return (it == ids_.end()) ? ids_.size()
		                          : std::distance(ids_.begin(), it);
	}

	nst::inplace_vector<value_type, Capacity> modules_;
	nst::inplace_vector<ModuleID, Capacity> ids_;
};

template <std::size_t N, class... ModuleBases> class MappedModuleRegistry {
	using Registry = ModuleRegistry<N, ModuleBases...>;

  public:
	constexpr static auto size = N;
	using value_type = typename Registry::value_type;

	template <class Module> struct Receipt {
		ModuleID id;
		Module &module;
	};

	enum class Error { LOCATION_OCCUPIED, REGISTRY_FAILED, POOL_EXHAUSTED };

	// Creates a Module at the given position. Returns the new id and a
	// reference to the module if successful, Error if not
	template <class Module, typename... Args>
	auto make(ModulePosition pos, Args &&...args)
	    -> nst::expected<Receipt<Module>, Error> {
		assert(pos.value < map_.size());

		auto &entry = map_[pos.value];
		if (!entry) {
			return Error::LOCATION_OCCUPIED;
		}
		auto m = arena::acquire_module<Module>(std::forward<Args>(args)...);
		if (!m) {
			return Error::POOL_EXHAUSTED;
		}
		auto new_id = registry_.add(m);
		if (!new_id) {
			return Error::REGISTRY_FAILED;
		}
		entry.emplace(MapEntry{*new_id, module_type<Module>, m});
		return Receipt<Module>{*new_id, *m};
	}

	bool erase(ModuleID id) {
		auto it = std::find_if(map_.begin(), map_.end(), [id](const auto &e) {
			return e.has_value() && e->id == id;
		});
		if (it == map_.end()) {
			return false;
		}
		auto &entry = *it;
		arena::release_module(entry->type, entry->ptr);
		entry.reset();
		registry_.erase(id);
		return true;
	}

	// Returns nullptr if not found.
	[[nodiscard]] auto get_module(ModuleID id) const -> const value_type * {
		auto it = registry_.find(id);
		if (it == registry_.cend()) {
			return nullptr;
		}
		return it;
	}

	// Returns nullptr if not found.
	[[nodiscard]] auto get_module(ModuleID id) -> value_type * {
		auto it = registry_.find(id);
		if (it == registry_.end()) {
			return nullptr;
		}
		return it;
	}

	[[nodiscard]] auto get_id(ModulePosition pos) const
	    -> std::optional<ModuleID> {
		const auto &entry = map_[pos.value];
		if (!entry.has_value()) {
			return std::nullopt;
		}
		return entry->id;
	}

	[[nodiscard]] const auto &operator[](ModuleID id) const {
		return registry_[id];
	}

	[[nodiscard]] auto &operator[](ModuleID id) { return registry_[id]; }

	[[nodiscard]] ModuleID operator[](ModulePosition pos) const {
		const auto &entry = map_[pos.value];
		assert(entry.has_value());
		return map_[pos.value]->id;
	}

	[[nodiscard]] ModuleType type_at(ModulePosition pos) const {
		const auto &entry = map_[pos.value];
		assert(entry.has_value());
		return entry->type;
	}

	[[nodiscard]] constexpr static auto capacity() { return size; }

  private:
	struct MapEntry {
		ModuleID id;
		ModuleType type;
		void *ptr;
	};
	using Map = std::array<std::optional<MapEntry>, size>;

	Registry registry_{};
	Map map_{};
};

} // namespace sndbx

#endif