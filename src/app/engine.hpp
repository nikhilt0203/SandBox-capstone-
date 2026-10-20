#ifndef SANDBOX_ENGINE_HPP_
#define SANDBOX_ENGINE_HPP_

#include <optional>

#include "audio/audio_engine.hpp"
#include "config/config.hpp"
#include "modules/factory/module_registry.hpp"
#include "modules/module_interfaces.hpp"

namespace sndbx {

namespace detail {
template <class InterfaceList> struct MakeModuleViewType {};

template <class... Interfaces>
struct MakeModuleViewType<nst::type_list<Interfaces...>> {
	using type = nst::poly_view<Interfaces...>;
};
} // namespace detail

using ModuleView = detail::MakeModuleViewType<ModuleInterfaces>::type;

using ModuleFactory = MappedModuleRegistry<ModuleView, limits::max_modules>;

struct ModuleConnection {
	ModulePosition src_pos;
	std::uint8_t output_idx;
	ModulePosition dst_pos;
	std::uint8_t input_idx;
};

class Engine {
  public:
	bool connect(ModulePosition src_pos, std::uint8_t output_idx,
	             ModulePosition dst_pos, std::uint8_t input_idx);

	bool disconnect(ModulePosition src_pos, std::uint8_t output_idx,
	                ModulePosition dst_pos, std::uint8_t input_idx);

	bool connect_first(ModulePosition src_pos, ModulePosition dst_pos);

	bool disconnect_first(ModulePosition src_pos, ModulePosition dst_pos);

	auto create_module(ModuleType type, ModulePosition pos)
	    -> std::optional<ModuleID>;

	bool delete_module(ModulePosition pos);

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     ModulePosition dst_pos) const;

	[[nodiscard]] bool connection_exists(ModulePosition src_pos,
	                                     std::uint8_t output_idx,
	                                     ModulePosition dst_pos,
	                                     std::uint8_t input_idx) const;

	[[nodiscard]] auto module_id(ModulePosition pos) const
	    -> std::optional<ModuleID> {
		return factory_.get_id(pos);
	}

	[[nodiscard]] const auto &get_module(ModuleID id) const {
		return factory_[id];
	}
	[[nodiscard]] auto &get_module(ModuleID id) { return factory_[id]; }

	[[nodiscard]] const auto &factory() const { return factory_; }
	[[nodiscard]] auto &factory() { return factory_; }

	[[nodiscard]] auto &connections() { return connections_; }
	[[nodiscard]] const auto &connections() const { return connections_; }

  private:
	ModuleFactory factory_;
	AudioGraph audio_graph_;
	nst::inplace_vector<ModuleConnection, limits::max_module_connections>
	    connections_;
};

} // namespace sndbx

#endif