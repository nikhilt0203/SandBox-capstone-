#ifndef SANDBOX_APP_HPP_
#define SANDBOX_APP_HPP_

#include <Arduino.h>

#include "audio/audio_engine.hpp"
#include "input_handler.hpp"
#include "logging.hpp"
#include "modules/module_registry.hpp"
#include "modules/module_types.hpp"
#include "nst/hardware/rotary_encoder.hpp"
#include "pinouts.hpp"
#include "sd_card.hpp"
#include "ui/led_matrix.hpp"
#include "ui/ui.hpp"
#include <nst/hardware/rotary_encoder.hpp>
#include <nst/hardware/trellis.hpp>
#include <nst/inplace_vector.hpp>

namespace sndbx::input {

inline static InputEventQueue event_queue;
void handle_event(const sndbx::InputEvent &e);

inline static auto keypad =
    nst::teensy::make_multitrellis<pinouts::trellis_addrs>(event_queue);

inline static auto knobs =
    nst::teensy::make_encoder_array(pinouts::encoder_pins, event_queue);

inline static auto buttons =
    nst::teensy::make_button_array(pinouts::button_pins, event_queue);

inline static std::optional<KeypadEvent> last_keypad_evt;

} // namespace sndbx::input

namespace sndbx::app {

void update();

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

class Engine {
public:
  using ModuleFactory =
      MappedModuleRegistry<limits::max_modules, audio::Patchable, Displayable,
                           Controllable>;

  struct ModuleConnection {
    ModuleID src_id;
    ModulePort src_port;
    ModuleID dst_id;
    ModulePort dst_port;
  };

  bool connect(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
               ModulePort dst_port) {
    if (src_id == dst_id || connections_.is_full()) {
      return false;
    }

    auto modules = get_patchable_pair(src_id, dst_id);
    if (!modules) {
      return false;
    }

    if (AudioError::NONE != audio::connect(*modules->first, src_port,
                                           *modules->second, dst_port,
                                           audio_graph_)) {
      return false;
    }

    connections_.push_back(
        ModuleConnection{src_id, src_port, dst_id, dst_port});
    return true;
  }

  bool disconnect(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
                  ModulePort dst_port) {
    if (src_id == dst_id) {
      return false;
    }

    auto modules = get_patchable_pair(src_id, dst_id);
    if (!modules) {
      return false;
    }

    if (AudioError::NONE != audio::disconnect(*modules->first, src_port,
                                              *modules->second, dst_port,
                                              audio_graph_)) {
      return false;
    }

    auto it = find_connection(src_id, src_port, dst_id, dst_port);
    assert(it != connections_.end());
    connections_.erase(it);
    return true;
  }

  [[nodiscard]] auto create_module(ModuleType type, std::size_t pos)
      -> std::optional<ModuleID> {
    if (!create_module_impl(type, pos, ModuleTypes::index_sequence{})) {
      return std::nullopt;
    }

    const auto id = factory_[pos];

    // initialize audio if module has an audio component
    auto &module = factory_[id];
    if (module.holds<audio::Patchable>() &&
        init_audio(module.get<audio::Patchable>()) != AudioError::NONE) {
      factory_.erase(id);
      return std::nullopt;
    }

    return id;
  }

  [[nodiscard]] std::optional<ModuleID> module_id(std::size_t pos) {
    return factory_.get_id(pos);
  }

  [[nodiscard]] bool delete_module(std::size_t pos) {
    auto id = factory_.get_id(pos);
    if (!id) {
      return false;
    }

    // remove from audio system if module has an audio component
    auto &module = factory_[*id];
    if (module.holds<audio::Patchable>()) {
      module.get<audio::Patchable>().unlink(audio_graph_);
    }

    factory_.erase(*id);
    return true;
  }

private:
  AudioError init_audio(audio::Patchable &p) {
    const auto prev_nodes_size = audio_graph_.nodes().size();
    const auto prev_patches_size = audio_graph_.patches().size();

    auto shrink = [](auto &c, std::size_t size) {
      while (size < c.size()) {
        c.pop_back();
      }
    };

    const auto error = p.link(audio_graph_);
    if (error != AudioError::NONE) {
      // restore previous graph state
      shrink(audio_graph_.nodes(), prev_nodes_size);
      shrink(audio_graph_.patches(), prev_patches_size);
    }
    return error;
  }

  auto find_connection(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
                       ModulePort dst_port) -> ModuleConnection * {
    return std::find_if(connections_.begin(), connections_.end(),
                        [src_id, src_port, dst_id, dst_port](const auto &c) {
                          return src_id == c.src_id && src_port == c.src_port &&
                                 dst_id == c.dst_id && dst_port == c.dst_port;
                        });
  }

  auto find_connection(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
                       ModulePort dst_port) const -> const ModuleConnection * {
    return std::find_if(connections_.cbegin(), connections_.cend(),
                        [src_id, src_port, dst_id, dst_port](const auto &c) {
                          return src_id == c.src_id && src_port == c.src_port &&
                                 dst_id == c.dst_id && dst_port == c.dst_port;
                        });
  }

  auto get_patchable_pair(ModuleID src, ModuleID dst) -> std::optional<
      std::pair<const audio::Patchable *, const audio::Patchable *>> const {
    auto m1 = factory_.get_module(src);
    if (!m1 || !m1->holds<audio::Patchable>()) {
      return std::nullopt;
    }
    auto m2 = factory_.get_module(dst);
    if (!m2 || !m2->holds<audio::Patchable>()) {
      return std::nullopt;
    }
    return std::make_pair(static_cast<const audio::Patchable *>(*m1),
                          static_cast<const audio::Patchable *>(*m2));
  }

  template <std::size_t... Is>
  bool create_module_impl(ModuleType type, std::size_t pos,
                          std::index_sequence<Is...>) {
    return (
        (ModuleType{Is} == type && factory_.make<ModuleTypes::get<Is>>(pos)) ||
        ...);
  }

  ModuleFactory factory_;
  AudioGraph audio_graph_;
  nst::inplace_vector<ModuleConnection, ModuleFactory::capacity()> connections_;
};

class App {
public:
  void update() {}

  bool connect(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
               ModulePort dst_port) {
    return engine_.connect(src_id, src_port, dst_id, dst_port);
  }

  bool disconnect(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
                  ModulePort dst_port) {
    return engine_.disconnect(src_id, src_port, dst_id, dst_port);
  }

private:
  Engine engine_;
};

inline static App instance;
inline static InputEventHandler input_handler{instance};

namespace input {
void handle_event(const sndbx::InputEvent &e) { std::visit(input_handler, e); }
} // namespace input

namespace app {
void update() { instance.update(); }
} // namespace app

} // namespace sndbx

#endif