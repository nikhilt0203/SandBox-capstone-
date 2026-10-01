#include "application.hpp"
#include <algorithm>

namespace sndbx {

void App::update() {}

void App::rotate_bank(std::int8_t amt) {
  const std::int8_t min =
      -1 * std::distance(&*module_bank.begin(), bank_window_.begin());
  const std::int8_t max =
      std::distance(bank_window_.end(), &*module_bank.end());
  bank_window_.slide(std::clamp(amt, min, max));
}

bool App::create_module(ModuleType type, ModulePosition pos) {
  // display
  return engine_.create_module(type, pos).has_value();
}

bool App::delete_module(ModulePosition pos) {
  // display
  return engine_.delete_module(pos);
}

bool App::connect(ModulePosition src_pos, std::uint8_t output_idx,
                  ModulePosition dst_pos, std::uint8_t input_idx) {
  // do display stuff
  return engine_.connect(src_pos, output_idx, dst_pos, input_idx);
}

bool App::disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                     ModulePosition dst_pos, std::uint8_t input_idx) {
  // do display stuff
  return engine_.disconnect(src_pos, output_idx, dst_pos, input_idx);
}

bool App::connect_first(ModulePosition src_pos, ModulePosition dst_pos) {
  // do display stuff
  return engine_.connect_first(src_pos, dst_pos);
}

bool App::disconnect_first(ModulePosition src_pos, ModulePosition dst_pos) {
  // do display stuff
  return engine_.disconnect_first(src_pos, dst_pos);
}

auto App::module_id(ModulePosition pos) const -> std::optional<ModuleID> {
  return engine_.module_id(pos);
}

void App::select(ModuleID id) { selected_module_ = engine_.get_module(id); }

void App::turn_module_knob(std::uint8_t idx, std::int8_t amt) {
  if (selected_module_.holds<Controllable>()) {
    selected_module_.get<Controllable>().change(idx, amt);
  }
}

} // namespace sndbx