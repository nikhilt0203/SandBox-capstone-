#ifndef SANDBOX_ENGINE_HPP_
#define SANDBOX_ENGINE_HPP_

#include <optional>

#include "audio.hpp"
#include "config/config.hpp"
#include "modules/factory/module_registry.hpp"
#include "modules/module_interfaces.hpp"
#include <nst/teensy/audio_guard.hpp>

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

class Engine {
  public:
	enum class Error {
		CAPACITY_REACHED,
		MODULE_POOL_EXHAUSTED,
		LOCATION_OCCUPIED,
		MODULE_AUDIO_FAILED,
		NONE
	};

	bool connect(ModulePosition src_pos, std::uint8_t output_idx,
	             ModulePosition dst_pos, std::uint8_t input_idx);

	bool disconnect(ModulePosition src_pos, std::uint8_t output_idx,
	                ModulePosition dst_pos, std::uint8_t input_idx);

	bool connect_first(ModulePosition src_pos, ModulePosition dst_pos);

	bool disconnect_first(ModulePosition src_pos, ModulePosition dst_pos);

	nst::expected<ModuleView, Error> create_module(ModuleType type,
	                                               ModulePosition pos) {
		if (modules_.size() == limits::max_modules) {
			return Error::CAPACITY_REACHED;
		}

		if (module_exists(pos)) {
			return Error::LOCATION_OCCUPIED;
		}

		if (!create_module_impl(type, pos, ModuleTypes::index_sequence{})) {
			return Error::MODULE_POOL_EXHAUSTED;
		}
		// link to audio graph if module is Patchable, delete module if failure
		const auto id = module_id(pos);
		auto &module = modules_[id];
		if (module.holds<Patchable>() &&
		    !module.get<Patchable>().link(audio_graph_)) {
			modules_.erase(id);
			return Error::MODULE_AUDIO_FAILED;
		}
		return module;
	}

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     ModulePosition dst_pos) const;

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     std::uint8_t output_idx,
	                                     ModulePosition dst_pos,
	                                     std::uint8_t input_idx) const;

	[[nodiscard]] const auto &modules() const { return modules_; }
	[[nodiscard]] auto &modules() { return modules_; }

	[[nodiscard]] auto &connections() { return connections_; }
	[[nodiscard]] const auto &connections() const { return connections_; }

	void save_current_patch() {
		// auto save_val = [this](auto v) {
		// 	preset_buffer_.push_back(std::byte{v});
		// };

		// for (std::size_t i{}; i < limits::grid_size; ++i) {
		// 	const auto pos = ModulePosition{i};
		// 	const auto id = get_module_id(pos);
		// 	if (!id) {
		// 		continue;
		// 	}

		// 	const auto type = factory_.type_at(pos);
		// 	const auto &module = factory_[*id];

		// 	save_val(type.value);
		// 	save_val(pos.value);
		// 	nst::invoke_if<Serializable>(
		// 	    module, [this](const auto &s) { s.serialize(preset_buffer_); });
		// }
		// for (const auto c : connections_) {
		// 	save_val(0);
		// 	save_val(c.src_pos.value);
		// 	save_val(c.output_idx);
		// 	save_val(c.dst_pos.value);
		// 	save_val(c.input_idx);
		// }
	}

	void delete_module(ModulePosition pos) {
		assert(module_exists(pos));
		auto &entry = module_map_[pos.value];
		arena::release_module(entry->type, entry->ptr);
		modules_.erase(entry->id);
		entry.reset();
	}

	[[nodiscard]] const auto &get_module(ModuleID id) const {
		return modules_[id];
	}

	[[nodiscard]] auto &get_module(ModuleID id) { return modules_[id]; }

	[[nodiscard]] ModuleID module_id(ModulePosition pos) const {
		const auto &entry = module_map_[pos.value];
		assert(entry.has_value());
		return entry->id;
	}

	[[nodiscard]] ModuleType type(ModulePosition pos) const {
		const auto &entry = module_map_[pos.value];
		assert(entry.has_value());
		return entry->type;
	}

	[[nodiscard]] bool module_exists(ModulePosition pos) const {
		return module_map_[pos.value].has_value();
	}

	template <class Module, typename... Args>
	bool create_module(ModulePosition pos, Args &&...args) {
		assert(pos.value < module_map_.size());
		auto module_ptr =
		    arena::acquire_module<Module>(std::forward<Args>(args)...);
		if (!module_ptr) {
			return false;
		}
		auto new_id = modules_.add(module_ptr);
		module_map_[pos.value] = MapEntry{*new_id, module_type<Module>,
		                          (void *)module_ptr};
		return true;
	}

  private:
	struct MapEntry {
		ModuleID id;
		ModuleType type;
		void *ptr;
	};

	using ModuleMap = std::array<std::optional<MapEntry>, limits::grid_size>;
	ModuleRegistry<ModuleView, limits::max_modules> modules_{};
	ModuleMap module_map_{};

	AudioGraph audio_graph_;
	nst::inplace_vector<ModuleConnection, limits::max_module_connections>
	    connections_;

	SerializationBuffer preset_buffer_;

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