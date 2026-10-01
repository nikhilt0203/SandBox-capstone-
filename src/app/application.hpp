#ifndef SANDBOX_APP_HPP_
#define SANDBOX_APP_HPP_

#include <Arduino.h>

#include "input_handler.hpp"
#include "io/pinouts.hpp"
#include "io/sd_card.hpp"
#include "nst/hardware/rotary_encoder.hpp"
#include "ui/led_matrix.hpp"
#include "ui/ui.hpp"
#include <nst/hardware/rotary_encoder.hpp>
#include <nst/hardware/trellis.hpp>
#include <nst/inplace_vector.hpp>
#include <nst/span.hpp>

#include "engine.hpp"
#include "modules/module_bank.hpp"

namespace sndbx::input {

inline void handle_event(const InputEvent &e);

inline static nst::inplace_vector<InputEvent, limits::max_input_evts>
    event_queue;

inline static auto keypad =
    nst::teensy::make_multitrellis<pinouts::trellis_addrs>(event_queue);

inline static auto knobs =
    nst::teensy::make_encoder_array(pinouts::encoder_pins, event_queue);

inline static auto buttons =
    nst::teensy::make_button_array(pinouts::button_pins, event_queue);

} // namespace sndbx::input

namespace sndbx::app {

inline void update();

inline void init() { Serial.begin(115200); }

inline void loop() {
  input::keypad.update();

  for (auto &knob : input::knobs) {
    knob.update();
  }

  while (!input::event_queue.is_empty()) {
    input::handle_event(input::event_queue.pop());
  }

  update();
}

} // namespace sndbx::app

namespace sndbx {

class App {
public:
  void update();

  bool create_module(ModuleType type, ModulePosition pos);
  bool delete_module(ModulePosition pos);

  bool connect(ModulePosition src_pos, std::uint8_t output_idx,
               ModulePosition dst_pos, std::uint8_t input_idx);

  bool disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                  ModulePosition dst_pos, std::uint8_t input_idx);

  bool connect_first(ModulePosition src_pos, ModulePosition dst_pos);
  bool disconnect_first(ModulePosition src_pos, ModulePosition dst_pos);

  void press_module(ModuleID id);
  void turn_module_knob(std::uint8_t idx, std::int8_t amt);

  void rotate_bank(std::int8_t amt);
  void select(ModuleID id);

  void clear_selection() { selected_module_ = {}; }

  bool has_selection() const { return selected_module_; }

  [[nodiscard]] auto module_id(ModulePosition pos) const
      -> std::optional<ModuleID>;

  [[nodiscard]] const auto &engine() const { return engine_; }

private:
  inline static constexpr ModuleBank module_bank =
      make_module_bank(ModuleBankTypes::index_sequence{});

  Engine engine_;

  ModuleView selected_module_{};

  nst::span<const ModuleBankEntry> bank_window_{module_bank.begin(),
                                          config::grid_cols};
};

/*
 * Global application instance
 */
inline static App application;

inline static InputEventHandler input_handler{application};

inline void app::update() { application.update(); }

inline void input::handle_event(const sndbx::InputEvent &e) {
  std::visit(input_handler, e);
}

} // namespace sndbx

#endif