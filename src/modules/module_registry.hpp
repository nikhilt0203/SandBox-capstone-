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

template <class NonOwningModuleView, std::size_t Capacity>
class ModuleRegistry {
  public:
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

	nst::inplace_vector<NonOwningModuleView, Capacity> modules_;
	nst::inplace_vector<ModuleID, Capacity> ids_;
};

// Module storage. A module's ModuleID can be retrieved with the module's
// ModulePosition, and its underlying NonOwningModuleView (template param) can
// be queried with its ModuleID.
template <class NonOwningModuleView, std::size_t Capacity>
class MappedModuleRegistry {
  public:
	template <class Module> struct Receipt {
		ModuleID id;
		Module &module;
	};

	enum class Error {
		CAPACITY_REACHED,
		LOCATION_OCCUPIED,
		MODULE_POOL_EXHAUSTED
	};

	// Creates a Module at the given position.
	// When successful, the expected type contains a receipt with a
	// reference to the new module and its id. Otherwise, it contains
	// the nested Error enum
	template <class Module, typename... Args>
	auto make(ModulePosition pos, Args &&...args)
	    -> nst::expected<Receipt<Module>, Error> {
		assert(pos.value < map_.size() && "Invalid module position.");

		if (registry_.size() == Capacity) {
			return Error::CAPACITY_REACHED;
		}

		auto &entry = map_[pos.value];
		if (!entry) {
			return Error::LOCATION_OCCUPIED;
		}

		auto module = arena::acquire_module<Module>(std::forward<Args>(args)...);
		if (!module) {
			return Error::MODULE_POOL_EXHAUSTED;
		}

		auto new_id = registry_.add(module);
		entry.emplace(MapEntry{*new_id, module_type<Module>, module});
		return Receipt<Module>{*new_id, *module};
	}

	bool erase(ModuleID id) {
		auto it = std::find_if(map_.begin(), map_.end(), [id](const auto &e) {
			return e && e->id == id;
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

	// returns nullptr if not found
	[[nodiscard]] auto get_module(ModuleID id) const
	    -> const NonOwningModuleView * {
		auto it = registry_.find(id);
		if (it == registry_.cend()) {
			return nullptr;
		}
		return it;
	}

	// returns nullptr if not found
	[[nodiscard]] auto get_module(ModuleID id) -> NonOwningModuleView * {
		auto it = registry_.find(id);
		if (it == registry_.end()) {
			return nullptr;
		}
		return it;
	}

	// std::nullopt if not found
	[[nodiscard]] auto get_id(ModulePosition pos) const
	    -> std::optional<ModuleID> {
		const auto &entry = map_[pos.value];
		if (!entry.has_value()) {
			return std::nullopt;
		}
		return entry->id;
	}

	// retrieve the module with the given id. id must be valid
	[[nodiscard]] const auto &operator[](ModuleID id) const {
		return registry_[id];
	}

	// retrieve the module with the given id. id must be valid
	[[nodiscard]] auto &operator[](ModuleID id) { return registry_[id]; }

	// retrieve the id of the module at the given position. a module
	// must exist at the position
	[[nodiscard]] ModuleID operator[](ModulePosition pos) const {
		const auto &entry = map_[pos.value];
		assert(entry.has_value());
		return map_[pos.value]->id;
	}

	// retrieve the type id of the module at the given position. a module
	// must exist at the position
	[[nodiscard]] ModuleType type_at(ModulePosition pos) const {
		const auto &entry = map_[pos.value];
		assert(entry.has_value());
		return entry->type;
	}

	// the number of modules that can be stored
	[[nodiscard]] constexpr static auto capacity() { return Capacity; }

  private:
	struct MapEntry {
		ModuleID id;
		ModuleType type;
		void *ptr;
	};
	using Map = std::array<std::optional<MapEntry>, capacity()>;

	ModuleRegistry<NonOwningModuleView, capacity()> registry_{};
	Map map_{};
};

} // namespace sndbx

#endif