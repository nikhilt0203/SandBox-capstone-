#ifndef SANDBOX_ENGINE_HPP_
#define SANDBOX_ENGINE_HPP_

#include <nst/expected.hpp>
#include <nst/poly_view.hpp>

#include "audio.hpp"
#include "config/config.hpp"
#include "modules/factory/module_map.hpp"
#include "modules/factory/module_pools.hpp"
#include "modules/module_interfaces.hpp"

namespace sndbx {

namespace detail {
template <class InterfaceList> struct ModuleView {};

template <class... Interfaces>
struct ModuleView<nst::type_list<Interfaces...>> {
	using type = nst::poly_view<Interfaces...>;
};
} // namespace detail

using ModuleView = detail::ModuleView<ModuleInterfaces>::type;

struct ModuleConnection {
	ModulePosition src_pos;
	std::uint8_t output_idx;
	ModulePosition dst_pos;
	std::uint8_t input_idx;
};

enum class Error {
	CAPACITY_REACHED,
	MODULE_POOL_EXHAUSTED,
	LOCATION_OCCUPIED,
	MODULE_AUDIO_FAILED,
	NONE
};

class Engine {
  public:
	bool connect(ModulePosition src_pos, std::uint8_t output_idx,
	             ModulePosition dst_pos, std::uint8_t input_idx);

	bool disconnect(ModulePosition src_pos, std::uint8_t output_idx,
	                ModulePosition dst_pos, std::uint8_t input_idx);

	bool connect_first(ModulePosition src_pos, ModulePosition dst_pos);

	bool disconnect_first(ModulePosition src_pos, ModulePosition dst_pos);

	bool has_module(ModulePosition pos) { return module_map_.is_occupied(pos); }

	nst::expected<ModuleView, Error> create_module(ModuleType type,
	                                               ModulePosition pos) {
		if (module_map_.is_full()) {
			return Error::CAPACITY_REACHED;
		}

		if (module_map_.is_occupied(pos)) {
			return Error::LOCATION_OCCUPIED;
		}

		if (!create_module_impl(type, pos, ModuleTypes::index_sequence{})) {
			return Error::MODULE_POOL_EXHAUSTED;
		}
		// link to audio graph if module is Patchable, delete module if failure
		auto &new_module = module_map_[pos];
		if (new_module.holds<Patchable>() &&
		    !new_module.get<Patchable>().link(audio_graph_)) {
			delete_module(pos);
			return Error::MODULE_AUDIO_FAILED;
		}
		return new_module;
	}

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     ModulePosition dst_pos) const;

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     std::uint8_t output_idx,
	                                     ModulePosition dst_pos,
	                                     std::uint8_t input_idx) const;

	[[nodiscard]] const auto &connections() const { return connections_; }

	void save_patch(std::string_view name) {}

	void delete_module(ModulePosition pos) {
		module_map_.remove(pos, module_pools::release);
	}

	[[nodiscard]] const auto &get_module(ModulePosition pos) const {
		return module_map_[pos];
	}

	[[nodiscard]] auto &get_module(ModulePosition pos) {
		return module_map_[pos];
	}

  private:
	AudioGraph audio_graph_;
	ModuleMap<ModuleView, limits::max_modules> module_map_{};

	nst::inplace_vector<ModuleConnection, limits::max_module_connections>
	    connections_;

	SerializationBuffer preset_buffer_;

	template <class Module, typename... Args>
	bool create_module(ModulePosition pos, Args &&...args) {
		if (auto module_ptr =
		        module_pools::acquire<Module>(std::forward<Args>(args)...)) {
			module_map_.add(module_ptr, pos);
			return true;
		}
		return false;
	}

	template <std::size_t... Is>
	bool create_module_impl(ModuleType type, ModulePosition pos,
	                        std::index_sequence<Is...>) {
		// short circuits on success
		return ((ModuleType{Is} == type &&
		         create_module<ModuleTypes::get<Is>>(pos)) ||
		        ...);
	}
};

} // namespace sndbx

#endif