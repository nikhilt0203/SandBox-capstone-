#ifndef SANDBOX_INPUT_HANDLER_HPP_
#define SANDBOX_INPUT_HANDLER_HPP_

#include <optional>

#include "event.hpp"
#include "logging.hpp"
#include "nst/hardware/trellis.hpp"
#include "timer.hpp"
#include <algorithm>
#include <nst/inplace_string.hpp>
#include <nst/span.hpp>

struct App;

class AppState {
public:
  virtual ~AppState() = default;
  virtual void handle_knob_evt(App &, const sndbx::KnobEvent &) = 0;
  virtual void handle_keypad_evt(App &, const sndbx::KeypadEvent &) = 0;
  virtual void handle_button_evt(App &, const sndbx::ButtonEvent &) = 0;
};

class EditMode final : public AppState {
public:
  void handle_knob_evt(App &app, const sndbx::KnobEvent &e) override {
    const auto [knob_idx, delta] = e;
    // app.modulate_param(knob_idx, delta, (id = selected_id));
    if (prev_keypad_evt_.has_value() && prev_keypad_evt_->data.key_num > 55) {
      const std::int8_t min =
          -1 * std::distance(window.begin(), &*module_bank.begin());
      const std::int8_t max = std::distance(window.end(), &*module_bank.end());
      window.slide(std::clamp(delta, min, max));
    }
  }

  void handle_keypad_evt(App &app, const sndbx::KeypadEvent &e) override {
    const auto key_num = e.data.key_num;
    Serial.printf("key %d pressed", key_num);
    constexpr static auto hold_ms = 1000u;
    constexpr static auto timeout_ms = 1500u;
    // const auto module_id = app.module_id(key_num);

    if (prev_keypad_evt_.has_value() &&
        prev_keypad_evt_->data.key_num != key_num &&
        e.time - prev_keypad_evt_->time < timeout_ms) {
      // const auto src_id = app.module_id(prev_keypad_evt_->data.key_num);
      // if (!app.connection_exists(src_id, module_id))
      //   app.connect(src_id, module_id);
      // else
      //   app.disconnect(src_id, module_id);
    }

    switch (e.data.edge) {
    case sndbx::KeypadEvent::Edge::RISING_EDGE:
      timer_.start();
    // if (module_id != 0)
    //  app.select_module(module_id);
    //  break;
    case sndbx::KeypadEvent::Edge::FALLING_EDGE:
      if (timer_.hasReached(hold_ms)) {
        // app.delete_module(module_id);
      }
      // app.handleTrellisFallingEdge(e);
      break;
    }

    prev_keypad_evt_ = e;
  }
  void handle_button_evt(App &app, const sndbx::ButtonEvent &e) override {}

private:
  template <typename T, typename Storage>
  void slide_span(nst::span<T> &span, const Storage &storage,
                  std::int32_t amt) {
    const std::int32_t min =
        -1 * std::distance(span.begin(), &*storage.begin());
    const std::int32_t max = std::distance(span.end(), &*storage.end());
    span.slide(std::clamp(amt, min, max));
  }

private:
  struct BankEntry {
    std::size_t type;
    std::string_view description;
  };

  Timer timer_;
  std::optional<sndbx::KeypadEvent> prev_keypad_evt_{};
  constexpr static std::array<BankEntry, 10> module_bank{};
  nst::span<const BankEntry> window{module_bank.begin(), 8};
};

class ViewMode final : public AppState {
public:
  void handle_knob_evt(App &app, const sndbx::KnobEvent &e) override {}
  void handle_keypad_evt(App &app, const sndbx::KeypadEvent &e) override {}
  void handle_button_evt(App &app, const sndbx::ButtonEvent &e) override {}
};

class InputHandler {
public:
  template <typename Event> using Callback = void (*)(const Event &);

  InputHandler(Callback<sndbx::KeypadEvent> a, Callback<sndbx::KnobEvent> b)
      : keypad_cb_{a}, knob_cb_{b} {}

  void operator()(const sndbx::KeypadEvent &evt) {
    // state_->handle_keypad_evt(app_, evt);
    keypad_cb_(evt);
  }

  void operator()(const sndbx::KnobEvent &evt) {
    // state_->handle_knob_evt(app_, evt);
    knob_cb_(evt);
  }

  void operator()(const sndbx::ButtonEvent &evt) {
    // state_->handle_button_evt(app_, evt);
  }

  template <typename State> void change_state() {
    state_ = &std::get<State>(states_);
  }

  [[nodiscard]] auto &event_queue() { return events_; }

private:
  // App &app_;
  Callback<sndbx::KeypadEvent> keypad_cb_;
  Callback<sndbx::KnobEvent> knob_cb_;

  std::tuple<EditMode, ViewMode> states_{};
  AppState *state_{&std::get<EditMode>(states_)};
  sndbx::InputEventQueue events_;
};

// template<typename ...States>
// class InputHandler {
// static_assert((std::is_base_of_v<State, States> && ...));
// public:
//   InputHandler(App &app) : m_App(app) {
//     // m_Buttons.enableRisingEdge();
//   }

//   void update() {
//     m_Trellis.update();
//     m_Encoders.update();
//     // m_Buttons.update();

//     while (m_Trellis.hasEvent()) {
//       m_CurrentState->onTrellisPress(m_App, m_Trellis.popEvent());
//     }
//     while (m_Encoders.hasEvent()) {
//       m_CurrentState->handle_knob_evt(m_App, m_Encoders.popEvent());
//     }
//     //sndbx::event::handle_evts(m_Buttons, buttons_handler);

//     // if (m_Buttons.hasEvent())  {
//     handle_button_evt(m_Buttons.popEvent());
//     }
//   }

//   [[nodiscard]] Trellis &trellis() { return m_Trellis; }

//   template <typename State> void change_state() {
//     m_CurrentState = &std::get<State>(m_States);
//   }

// private:
//   void handle_button_evt(const sndbx::event::ButtonPress &buttonEvent) {
//     if (buttonEvent.edge == sndbx::event::Edge::FALLING_EDGE) {
//       return;
//     }
//     // switch (buttonEvent.buttonNum)
//     // {
//     //   case 0: //toggleEditMode();
//     //   case 1: //app.saveCurrentPatch()
//     //   case 2: //app.loadPatch()
//     //   case 3: //app.shift()
//     // }
//   }

// private:
//   App &m_App;
//   Encoders m_Encoders;
//   Trellis m_Trellis;
//   State *m_CurrentState{&std::get<0>(m_States)};
//   std::tuple<States...> m_States{};
// };

#endif