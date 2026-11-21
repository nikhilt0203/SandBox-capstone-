#ifndef SANDBOX_MODULE_REGISTRY_HPP_
#define SANDBOX_MODULE_REGISTRY_HPP_

#include <algorithm>
#include <cstddef>
#include <optional>

#include "nst/expected.hpp"
#include "nst/inplace_vector.hpp"
#include "nst/poly_view.hpp"
#include "nst/strong_alias.hpp"

#include "module_id.hpp"
#include "module_pools.hpp"
#include "module_position.hpp"

namespace sndbx {

template <class ModuleView, std::size_t Capacity> class ModuleRegistry {
  public:
	template <class Module>
	[[nodiscard]] auto add(Module *m) -> std::optional<ModuleID> {
		if (modules_.is_full()) {
			return std::nullopt;
		}
		const auto id = id_gen_.next_id();
		ids_.push_back(id);
		modules_.emplace_back(m);
		return id;
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

	[[nodiscard]] const auto find(ModuleID id) const {
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

	[[nodiscard]] const auto begin() const { return modules_.cbegin(); }
	[[nodiscard]] const auto end() const { return modules_.cend(); }
	[[nodiscard]] const auto cbegin() const { return begin(); }
	[[nodiscard]] const auto cend() const { return end(); }
	[[nodiscard]] auto begin() { return modules_.begin(); }
	[[nodiscard]] auto end() { return modules_.end(); }

	[[nodiscard]] const auto &ids() const { return ids_; }
	[[nodiscard]] auto size() const { return modules_.size(); }

  private:
	std::size_t index_of(ModuleID id) const {
		const auto it = std::find(ids_.begin(), ids_.end(), id);
		return (it == ids_.end()) ? ids_.size()
		                          : std::distance(ids_.begin(), it);
	}

	struct {
		ModuleID next_id() { return ModuleID{++last_id}; }
		std::size_t last_id{};
	} id_gen_;

	nst::inplace_vector<ModuleView, Capacity> modules_;
	nst::inplace_vector<ModuleID, Capacity> ids_;
};

} // namespace sndbx

#endif