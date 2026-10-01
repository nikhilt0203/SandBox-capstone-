#ifndef SANDBOX_ENGINE_HPP_
#define SANDBOX_ENGINE_HPP_

#include "audio/audio_engine.hpp"
#include "config/config.hpp"
#include "modules/controllable.hpp"
#include "modules/displayable.hpp"
#include "modules/module_registry.hpp"
#include "modules/pressable.hpp"
#include <optional>

namespace sndbx {

using ModuleFactory =
    MappedModuleRegistry<limits::max_modules, audio::Patchable, Displayable,
                         Controllable, Pressable>;

using ModuleView = ModuleFactory::value_type;

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

  bool connection_exists(ModulePosition src_pos, ModulePosition dst_pos) const;
  bool connection_exists(ModulePosition src_pos, std::uint8_t output_idx,
                         ModulePosition dst_pos, std::uint8_t input_idx) const;

  [[nodiscard]] auto create_module(ModuleType type, ModulePosition pos)
      -> std::optional<ModuleID>;

  [[nodiscard]] bool delete_module(ModulePosition pos);

  [[nodiscard]] auto module_id(ModulePosition pos) const
      -> std::optional<ModuleID> {
    return factory_.get_id(pos);
  }

  [[nodiscard]] auto get_module(ModuleID id) { return factory_[id]; }

private:
  ModuleFactory factory_;
  AudioGraph audio_graph_;
  nst::inplace_vector<ModuleConnection, limits::max_module_connections>
      connections_;
};

} // namespace sndbx

#endif