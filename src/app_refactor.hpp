#ifndef SANDBOX_APP_HPP_
#define SANDBOX_APP_HPP_

#include <Arduino.h>

#include "encoders.hpp"
#include "engine/builder_interface.hpp"
#include "engine/module_builder.hpp"
#include "engine/patching.hpp"
#include "final_refactor/module_registry.hpp"
#include "input_handler.hpp"
#include "keyboard_manager.hpp"
#include "logging.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include "sd_card.hpp"
#include "sequencer_manager.hpp"
#include "timer.hpp"
#include "ui/led_matrix.hpp"
#include "ui/tft_display.hpp"
#include "ui/ui.hpp"
#include <debug.hpp>
#include <nst/hardware/rotary_encoder.hpp>
#include <nst/hardware/trellis.hpp>
#include <nst/inplace_vector.hpp>

#include "final_refactor/audio_engine.hpp"
#include "final_refactor/module_registry.hpp"

namespace sndbx {
using Keypad =
    nst::teensy::MultiTrellis<config::i2c::trellis_addrs, InputEventQueue>;
using Knob = nst::teensy::RotaryEncoder<InputEventQueue>;
using Button = nst::teensy::Button<InputEventQueue>;

using ButtonArray = std::array<Button, config::hardware::num_buttons>;
using KnobArray = std::array<Knob, config::hardware::num_encoders>;
} // namespace sndbx

struct AppContext {
  enum class State {
    Idle,
    Selecting,
    Patching,
    Creating,
    Deleting,
    Displaying
  };

  enum class Mode { Edit, View };

  Mode mode{Mode::Edit};
  State state{State::Idle};

  Module *selected_module{};
  Displayable *selected_displayable{};
  Controllable *selected_controllable{};
  Animatable *selected_animatable{};

  // sndbx::KeypadEventQueue keypad_events{};
};

struct ModuleBankDisplay {
  nst::inplace_vector<std::uint32_t, sndbx::ModuleBank::size> colors;
  std::size_t start_index{0U};

  ModuleBankDisplay() {
    for (const auto &info : sndbx::engine::bankInfos) {
      colors.push_back(info.color);
    }
  }
};

struct Engine {
  AudioGraph audioGraph;
  ModuleBuilder builder;
  KeyboardManager kbd_manager;
  SequencerManager seq_manager;
};

inline AppContext AppContext_;

class LEDMatrixManager {
public:
  LEDMatrixManager(LEDMatrixDisplay &ledMatrix, Engine &engine,
                   AppContext &context)
      : m_LEDMatrix{ledMatrix}, m_Engine{engine}, m_AppContext(context) {}

  void draw_connection_between(sndbx::grid::Position srcPos,
                               sndbx::grid::Position destPos) {
    const auto moduleDisplay = m_Engine.builder.get<Displayable>(srcPos);

    const std::uint32_t wireColor =
        moduleDisplay
            ? sndbx::color::changeBrightness(moduleDisplay->displayColor(), 0.1)
            : 0x404040;

    auto currentRow = srcPos.row;
    auto currentCol = srcPos.col;

    while (currentRow != destPos.row) {
      if (currentRow > destPos.row) {
        --currentRow;
      } else if (currentRow < destPos.row) {
        ++currentRow;
      }
      m_LEDMatrix.drawPixel(currentRow, srcPos.col, wireColor);
    }

    while (currentCol != destPos.col) {
      if (currentCol > destPos.col) {
        --currentCol;
      } else if (currentCol < destPos.col) {
        ++currentCol;
      }
      m_LEDMatrix.drawPixel(currentRow, currentCol, wireColor);
    }
  }

  void draw_all_connections() {
    m_LEDMatrix.clear();

    for (std::uint8_t i{}; i < m_ModuleDisplays.size(); ++i) {
      const auto position = sndbx::grid::toPosition(i);

      auto module = m_Engine.builder.get<Module>(position);
      if (!module) {
        continue;
      }

      for (const auto &port : module->outputs()) {
        const auto connectedModule = port.connectedModule;
        if (!connectedModule) {
          continue;
        }

        if (auto destPos = getPosition(connectedModule)) {
          draw_connection_between(position, *destPos);
        }
      }
    }

    m_Dirty = true;
  }

  void place_module(Displayable *module, sndbx::grid::Position pos) {
    m_ModuleDisplays.at(pos.index()) = module;
    m_LEDMatrix.drawPixel(pos, module->ledColor());
    m_Dirty = false;
  }

  void render_frame() {
    if (m_Dirty) {
      clear();
      draw_all_connections();
      constexpr static auto numColors = sndbx::ModuleBank::size;
      sndbx::ui::draw<ModuleBank<numColors>>(m_LEDMatrix,
                                             m_ModuleBankDisplay.colors,
                                             m_ModuleBankDisplay.start_index);
    }

    for (std::size_t i{}; i < m_ModuleDisplays.size(); ++i) {
      const auto module = m_ModuleDisplays[i];
      if (!module) {
        continue;
      }

      auto moduleColor = module->ledColor();
      // brighten color if the module is selected
      if (m_AppContext.selected_displayable == module) {
        moduleColor = sndbx::color::blend(moduleColor, 0xDDDDFF, 0.1);
      }

      const auto modulePosition =
          sndbx::grid::toPosition(static_cast<std::uint8_t>(i));
      m_LEDMatrix.drawPixel(modulePosition, moduleColor);
    }

    m_LEDMatrix.renderFrame();
    m_Dirty = false;
  }

  void clear() { m_LEDMatrix.clear(); }

  void remove_module_display(sndbx::grid::Position pos) {
    m_ModuleDisplays.at(pos.index()) = nullptr;
    m_Dirty = true;
  }

  void mark_dirty() { m_Dirty = true; }

  [[nodiscard]] bool is_updated() const { return m_Dirty; }

private:
  [[nodiscard]] std::optional<sndbx::grid::Position> getPosition(Module *m) {
    for (const auto &entry : m_Engine.builder.registry()) {
      if (entry.module == m) {
        return entry.position;
      }
    }
    return std::nullopt;
  }

private:
  LEDMatrixDisplay &m_LEDMatrix;
  Engine &m_Engine;
  AppContext &m_AppContext;
  bool m_Dirty{true};
  std::array<Displayable *, sndbx::grid::totalCells> m_ModuleDisplays{};

public:
  ModuleBankDisplay m_ModuleBankDisplay;
};

class ScreenManager {
public:
  ScreenManager(TFT &tft, Engine &engine, AppContext &context)
      : m_TFT(tft), m_Engine(engine), m_AppContext(context) {}

  void display_splash() { sndbx::ui::clearAndDraw<SplashScreen>(m_TFT); }

  void display_module(Displayable *moduleDisplay, Module *module) {
    sndbx::ui::clearAndDraw<ModuleDisplay>(
        m_TFT, moduleDisplay->displayName(), moduleDisplay->displayColor(),
        moduleDisplay->controlNames(), moduleDisplay->normalizedControlValues(),
        moduleDisplay->inputNames(), input_module_colors(module),
        moduleDisplay->outputNames(), output_module_colors(module));
    dirty_ = true;
  }

  void display_selected_module() {
    if (auto animatable = m_AppContext.selected_animatable) {
      animatable->drawNext(m_TFT.currentFrame());
      m_TFT.setFrameAvailable(true);
      return;
    }

    if (dirty_) {
      if (auto selected = m_AppContext.selected_displayable) {
        display_module(selected, m_AppContext.selected_module);
      }
    }
  }

  void display_error(ModuleBuilder::Error error) {
    using namespace sndbx;
    switch (error) {
    case ModuleBuilder::Error::INVALID_POSITION:
      ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't place module here");
      break;
    case ModuleBuilder::Error::REGISTRY_FULL:
      ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create more modules");
      break;
    default:
      break;
    }
  }

  void display_error(const nst::string32_t &message) {
    sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, message.view());
    dirty_ = false;
  }

  void
  display_max_module_error(const sndbx::engine::ModuleBankEntry &moduleInfo) {
    sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create another");
    Text text{
        0, 150, moduleInfo.name, moduleInfo.color, 1, m_TFT.currentFrame()};
    text.centerX();
    sndbx::ui::draw(text, m_TFT);
    dirty_ = false;
  }

  void render_frame() { m_TFT.renderFrame(); }

  void mark_dirty() { dirty_ = true; }

  [[nodiscard]] bool is_updated() const { return dirty_; }

private:
  void fill_port_colors(nst::vector_8U<std::uint32_t> &portColorStorage,
                        const Module::PortArray &ports) {
    portColorStorage.clear();
    for (const auto &port : ports) {
      if (!port.connectedModule) {
        portColorStorage.push_back(0);
        continue;
      }

      const auto connectedID = port.connectedModule->id();

      if (auto connectedModule =
              m_Engine.builder.get<Displayable>(connectedID)) {
        portColorStorage.push_back(connectedModule->displayColor());
      }
    }
  }

  [[nodiscard]] const nst::vector_8U<std::uint32_t> &
  input_module_colors(Module *parent) {
    fill_port_colors(input_colors_, parent->inputs());
    return input_colors_;
  }

  [[nodiscard]] const nst::vector_8U<std::uint32_t> &
  output_module_colors(Module *parent) {
    fill_port_colors(output_colors_, parent->outputs());
    return output_colors_;
  }

private:
  TFT &m_TFT;
  Engine &m_Engine;
  AppContext &m_AppContext;
  bool dirty_{true};
  nst::vector_8U<std::uint32_t> input_colors_;
  nst::vector_8U<std::uint32_t> output_colors_;
};

void on_knob_turn(const sndbx::KnobEvent &);

inline Engine Engine_;

namespace sndbx {

inline static InputEventQueue event_queue;

inline static std::optional<KeypadEvent> last_keypad_evt;

inline auto keypad =
    nst::teensy::make_multitrellis<config::i2c::trellis_addrs>(event_queue);

inline auto knobs =
    nst::teensy::make_encoder_array(config::pinouts::encoder_pins, event_queue);

inline auto buttons =
    nst::teensy::make_button_array(config::pinouts::button_pins, event_queue);

} // namespace sndbx

struct Display {
  TFT screen;
  LEDMatrixDisplay ledMatrix;
  LEDMatrixManager led_manager{ledMatrix, Engine_, AppContext_};
  ScreenManager screen_manager{screen, Engine_, AppContext_};

  Display(Adafruit_MultiTrellis &trellis) : ledMatrix(trellis) {}
};

inline Display Display_{sndbx::keypad.multitrellis()};

inline void select_module(const ModuleBuilder::ModuleEntry *entry) {
  AppContext_.selected_module = entry->module;
  AppContext_.selected_displayable = entry->displayable;
  AppContext_.selected_controllable = entry->controllable;
  AppContext_.selected_animatable = entry->animatable;
}

inline void init_module(sndbx::grid::Position pos) {
  auto &builder = Engine_.builder;
  const auto entry = builder.getModuleEntry(pos);
  assert(entry);

  if (const auto moduleDisplay = entry->displayable) {
    Display_.led_manager.place_module(moduleDisplay, pos);
    Display_.screen_manager.mark_dirty();
  }

  select_module(entry);
}

struct App {};
App a;

namespace sndbx::app {

inline void turn_bank(int delta) {
  auto &bankDisplay = Display_.led_manager.m_ModuleBankDisplay;

  const auto newStartIndex =
      (static_cast<int>(bankDisplay.start_index) + delta) %
      static_cast<int>(bankDisplay.colors.size());

  bankDisplay.start_index = newStartIndex;
  Display_.led_manager.mark_dirty();
}

[[nodiscard]] inline std::size_t
bank_index(const sndbx::grid::Position &bankPos) {
  const auto &bankDisplay = Display_.led_manager.m_ModuleBankDisplay;
  return (bankPos.index() - sndbx::grid::bankStart + bankDisplay.start_index) %
         bankDisplay.colors.size();
}

inline void on_knob_turn(const sndbx::KnobEvent &evt) {
  Serial.println(millis());
  // Serial.printf("Encoder %d turned %d", num, delta);
  if (last_keypad_evt.has_value() &&
      sndbx::grid::is_bank(last_keypad_evt->data.key_num)) {
    turn_bank(evt.delta);
    return;
  }

  if (auto selected_module = AppContext_.selected_controllable) {
    selected_module->changeControl(evt.encoder_num, evt.delta);
    Display_.screen_manager.mark_dirty();
    Display_.led_manager.mark_dirty();
  }
}

inline void init() {
  Serial.begin(115200);
  Display_.led_manager.clear();
  Display_.screen_manager.display_splash();
  sndbx::sdcard::init();
}

inline bool delete_module(const sndbx::grid::Position &pos);

inline bool add_step(Sequencer &sequencer) {
  auto data = Engine_.seq_manager.addStep(sequencer, Engine_.builder);
  if (!data) {
    return false;
  }
  Display_.led_manager.place_module(data->step, data->position);
  return true;
}

inline bool subtract_step(Sequencer &sequencer) {
  return Engine_.seq_manager.subtractStep(sequencer, delete_module);
}

inline bool add_key(Keyboard &keyboard) {
  if (auto keyData = Engine_.kbd_manager.addKey(keyboard, Engine_.builder)) {
    Display_.led_manager.place_module(keyData->key, keyData->position);
    return true;
  }
  return false;
}

inline bool subtract_key(Keyboard &keyboard) {
  return Engine_.kbd_manager.subtractKey(keyboard, delete_module);
}

inline void change_scale(Keyboard::Scale scale, std::uint32_t keyboardID) {
  Engine_.kbd_manager.changeScale(scale, keyboardID);
}

inline void handle_delete_module(const sndbx::grid::Position &pos) {
  auto &keyboards = Engine_.kbd_manager;
  auto &sequencers = Engine_.seq_manager;

  if (keyboards.isKeyboardAt(pos)) {
    keyboards.deleteKeyboard(pos, delete_module);
  }
  if (sequencers.isSequencerAt(pos)) {
    sequencers.deleteSequencer(pos, delete_module);
  }
  if (keyboards.isKeyAt(pos) || sequencers.isStepAt(pos)) {
    return;
  } // keys & steps deleted when parent is deleted

  delete_module(pos);
}

inline void init_sequencer(Sequencer *sequencer, grid::Position pos) {
  assert(sequencer);

  sequencer->setAddStepCallback(add_step);
  sequencer->setSubtractStepCallback(subtract_step);
  Engine_.seq_manager.addSequencer(sequencer->id(), pos);
  init_module(pos);
  for (std::size_t steps{}; steps < 8; ++steps) {
    sequencer->changeControl(0, 1);
  }
}

inline void init_keyboard(Keyboard *keyboard, grid::Position pos) {
  assert(keyboard);

  keyboard->setAddKeyCallback(add_key);
  keyboard->setSubtractKeyCallback(subtract_key);
  keyboard->setScaleChangeCallback(change_scale);
  init_module(pos);
  Engine_.kbd_manager.addKeyboard(keyboard->id(), pos);
}

inline bool delete_module(const sndbx::grid::Position &pos) {
  const auto module = Engine_.builder.get<Module>(pos);
  if (!module) {
    return false;
  }

  patch::disconnectAll(Engine_.audioGraph, module);

  if (!Engine_.builder.destroy(pos)) {
    return false;
  }

  Display_.led_manager.remove_module_display(pos);
  return true;
}

inline void display_create_error(ModuleBuilder::Error error,
                                 std::size_t bank_index) {
  if (error == ModuleBuilder::Error::POOL_EXHAUSTED) {
    Display_.screen_manager.display_max_module_error(
        engine::bankInfos.at(bank_index));
  } else {
    Display_.screen_manager.display_error(error);
  }
}

inline void create_keyboard(const grid::Position &pos, ModuleBuilder &builder) {
  const auto result = builder.make<Keyboard>(pos);
  if (result) {
    auto keyboard = *result;
    init_keyboard(keyboard, pos);
    for (std::size_t keys{}; keys < 8; ++keys) {
      keyboard->changeControl(0, 1);
    }
  } else {
    display_create_error(result.error(), sndbx::bank_index<Keyboard>);
  }
}

inline void create_sequencer(const grid::Position &pos,
                             ModuleBuilder &builder) {
  const auto result = builder.make<Sequencer>(pos);
  if (result) {
    init_sequencer(result.value(), pos);
  } else {
    display_create_error(result.error(), sndbx::bank_index<Sequencer>);
  }
}

inline void create_module(std::size_t bank_index, const grid::Position &pos,
                          ModuleBuilder &builder) {
  switch (bank_index) {
  case sndbx::bank_index<Keyboard>:
    create_keyboard(pos, builder);
    break;
  case sndbx::bank_index<Sequencer>:
    create_sequencer(pos, builder);
    break;
  default: {
    const auto error =
        engine::createModuleFromBankIndex(bank_index, pos, builder);
    if (error == ModuleBuilder::Error::NONE) {
      init_module(pos);
    } else {
      display_create_error(error, bank_index);
    }
  }
  }
  AppContext_.state = AppContext::State::Displaying;
}

//========================================================================================================================
// UI
//========================================================================================================================

//========================================================================================================================
// Patching
//========================================================================================================================

inline bool handle_disconnect(Module *src, Module *dest) {
  return sndbx::patch::disconnectFirstConnection(Engine_.audioGraph, src, dest);
}

inline bool handle_connect(Module *src, Displayable *srcDisplay, Module *dest,
                           Displayable *destDisplay) {
  const auto output = sndbx::patch::firstAvailablePort(src->outputs());
  const auto input = sndbx::patch::firstAvailablePort(dest->inputs());
  if (!output || !input) {
    return false;
  }

  bool connectSuccess =
      sndbx::patch::connect(Engine_.audioGraph, src, *output, dest, *input);
  if (!connectSuccess) {
    return false;
  }

  if (!srcDisplay || !destDisplay) {
    return connectSuccess;
  }

  ui::clearAndDraw<PatchDisplayPage>(
      Display_.screen, srcDisplay->displayName(), destDisplay->displayName(),
      srcDisplay->outputNames().at(*output),
      destDisplay->inputNames().at(*input), srcDisplay->displayColor(),
      destDisplay->displayColor());

  return true;
}

inline void clear_module_selections(AppContext &ctx);
inline void patch_handler(const ModuleBuilder::ModuleEntry &src,
                          const ModuleBuilder::ModuleEntry &dst) {

  const auto src_module = src.module;
  const auto dst_module = dst.module;

  const bool modified = sndbx::patch::connectionExists(src_module, dst_module)
                            ? handle_disconnect(src_module, dst_module)
                            : handle_connect(src.module, src.displayable,
                                             dst_module, dst.displayable);

  if (modified) {
    clear_module_selections(AppContext_);
    // LOG("setting to patching");
    AppContext_.state = AppContext::State::Patching;
    Display_.led_manager.draw_all_connections();
  }
}

//========================================================================================================================
// Selection dispatch
//========================================================================================================================

inline void clear_module_selections(AppContext &ctx) {
  ctx.selected_module = nullptr;
  ctx.selected_displayable = nullptr;
  ctx.selected_controllable = nullptr;
  ctx.selected_animatable = nullptr;
}

inline void handle_rising_edge(std::uint16_t key_num) {
  const auto &builder = Engine_.builder;
  const auto &position = sndbx::grid::toPosition(key_num);

  const auto entry = builder.getModuleEntry(position);
  if (!entry) {
    return;
  }

  select_module(entry);

  if (auto pressable = entry->pressable) {
    pressable->onRisingEdge();
    Display_.led_manager.mark_dirty();
  }

  if (entry->displayable) {
    AppContext_.state = AppContext::State::Displaying;
    Display_.screen_manager.mark_dirty();
  }

  if (entry->animatable) {
    AppContext_.state = AppContext::State::Displaying;
  }
}

inline void handle_falling_edge(std::uint16_t key_num) {
  if (auto pressable =
          Engine_.builder.get<Pressable>(sndbx::grid::toPosition(key_num))) {
    pressable->onFallingEdge();
    Display_.led_manager.mark_dirty();
  }
}

inline void delete_module_at(std::uint16_t key_num) {
  handle_delete_module(sndbx::grid::toPosition(key_num));
  clear_module_selections(AppContext_);
}

inline bool handle_double_press(const sndbx::KeypadEvent &first,
                                const sndbx::KeypadEvent &second) {
  const auto first_pos = sndbx::grid::toPosition(first.data.key_num);
  const auto second_pos = sndbx::grid::toPosition(second.data.key_num);

  const bool patch_command = sndbx::grid::isBuildableArea(first_pos) &&
                             sndbx::grid::isBuildableArea(second_pos);

  const bool create_command = sndbx::grid::isBankArea(first_pos) &&
                              sndbx::grid::isBuildableArea(second_pos);

  if (patch_command) {
    if (auto src = Engine_.builder.getModuleEntry(first_pos); !src) {
      return false;
    } else if (auto dst = Engine_.builder.getModuleEntry(second_pos); !dst) {
      return false;
    } else {
      patch_handler(*src, *dst);
      return true;
    }
  } else if (create_command) {
    create_module(bank_index(first_pos), second_pos, Engine_.builder);
    return true;
  } else {
    return false;
  }
}

inline void on_keypad_press(const sndbx::KeypadEvent &evt) {
  using namespace sndbx;

  constexpr static auto long_press_thresh_ms = 1500u;
  constexpr static auto dbl_press_thresh_ms = 1800u;
  const auto &[edge, key_num, _] = evt.data;

  if (last_keypad_evt.has_value() &&
      AppContext_.mode == AppContext::Mode::Edit) {
    if (key_num != last_keypad_evt->data.key_num &&
        evt.time - last_keypad_evt->time < dbl_press_thresh_ms) {
      handle_double_press(*last_keypad_evt, evt);
    }

    if (edge == KeypadEvent::Edge::FALLING_EDGE &&
        last_keypad_evt->data.edge == KeypadEvent::Edge::RISING_EDGE &&
        evt.time - last_keypad_evt->time >= long_press_thresh_ms) {
      delete_module_at(key_num);
    }
  } else {
    switch (edge) {
    case KeypadEvent::Edge::RISING_EDGE:
      handle_rising_edge(key_num);
      break;
    case KeypadEvent::Edge::FALLING_EDGE:
      handle_falling_edge(key_num);
      break;
    }
  }
}

inline float processorUsage() {
  float total{};
  for (const auto &entry : Engine_.builder.registry()) {
    total += entry.module->audio().processorUsage();
  }
  return total;
}

InputHandler input_handler{on_keypad_press, on_knob_turn};

inline void read_inputs() {
  sndbx::keypad.update();

  for (auto &knob : sndbx::knobs) {
    knob.update();
  }
}

inline void update_state() {
  while (!sndbx::event_queue.is_empty()) {
    std::visit(input_handler, sndbx::event_queue.pop());
  }
  if (AppContext_.state == AppContext::State::Displaying)
    Display_.screen_manager.display_selected_module();
}

inline void render_display() {
  Display_.led_manager.render_frame();
  Display_.screen_manager.render_frame();
}

inline void loop() {
  read_inputs();
  update_state();
  render_display();
}

} // namespace sndbx::app

namespace sndbx {

struct ModuleConnection {
  ModuleID src_id;
  ModulePort src_port;
  ModuleID dst_id;
  ModulePort dst_port;

  ModuleConnection(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
                   ModulePort dst_port)
      : src_id{src_id}, src_port{src_port}, dst_id{dst_id}, dst_port{dst_port} {
  }
};

class Engine {
public:
  constexpr static auto null_id = ModuleID{0};

  using ModuleFactory =
      MappedModuleRegistry<config::audio::max_modules, Patchable, Displayable,
                           Controllable, Animatable, Serializable>;

  bool connect(ModuleID src_id, ModulePort src_port, ModuleID dst_id,
               ModulePort dst_port) {
    if (connections_.is_full()) {
      return false;
    }

    if (src_id == dst_id) {
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

    connections_.emplace_back(src_id, src_port, dst_id, dst_port);
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
    assert(it != connections_.end() && "Factory out of sync with audio graph");
    connections_.erase(it);
    return true;
  }

  [[nodiscard]] auto create_module(ModuleType type, std::size_t pos)
      -> std::optional<ModuleID> {
    if (!create_module_impl(type, pos, ModuleTypes::index_sequence{})) {
      return std::nullopt;
    }

    const auto id = factory_[pos];
    auto &module = factory_[id];
    if (module.holds<Patchable>() &&
        init_audio(module.get<Patchable>()) != AudioError::NONE) {
      factory_.erase(id);
      return std::nullopt;
    }

    return id;
  }

  [[nodiscard]] ModuleID module_id(std::size_t pos) {
    auto id = factory_.get_id(pos);
    if (!id) {
      return null_id;
    }
    return *id;
  }

  [[nodiscard]] bool delete_module(std::size_t pos) {
    auto id = factory_.get_id(pos);
    if (!id) {
      return false;
    }
    auto &module = factory_[*id];
    if (module.holds<Patchable>()) {
      module.get<Patchable>().unlink(audio_graph_);
    }
    factory_.erase(*id);
    return true;
  }

private:
  template <std::size_t... Is>
  bool create_module_impl(ModuleType type, std::size_t pos,
                          std::index_sequence<Is...>) {
    return (
        (ModuleType{Is} == type && factory_.make<ModuleTypes::get<Is>>(pos)) ||
        ...);
  }

  AudioError init_audio(Patchable &p) {
    const auto prev_nodes_size = audio_graph_.nodes().size();
    const auto prev_patches_size = audio_graph_.patches().size();

    auto shrink = [](auto &c, std::size_t size) {
      while (size < c.size()) {
        c.pop_back();
      }
    };

    const auto result = p.link(audio_graph_);
    if (result != AudioError::NONE) {
      // remove any additions made in link()
      shrink(audio_graph_.nodes(), prev_nodes_size);
      shrink(audio_graph_.patches(), prev_patches_size);
    }
    return result;
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

  ModuleFactory factory_;
  AudioGraph audio_graph_;
  nst::inplace_vector<ModuleConnection, ModuleFactory::capacity()> connections_;

  auto get_patchable_pair(ModuleID src, ModuleID dst)
      -> std::optional<std::pair<const Patchable *, const Patchable *>> const {
    auto m1 = factory_.get_module(src);
    if (!m1 || !m1->holds<Patchable>()) {
      return std::nullopt;
    }
    auto m2 = factory_.get_module(dst);
    if (!m2 || !m2->holds<Patchable>()) {
      return std::nullopt;
    }
    return std::make_pair(static_cast<const Patchable *>(*m1),
                          static_cast<const Patchable *>(*m2));
  }
};

class App {
public:
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

} // namespace sndbx

#endif