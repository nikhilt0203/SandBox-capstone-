#include "engine/module_builder.hpp"
#include "engine/patching.hpp"
#include "engine/builder_interface.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include "trellis.hpp"
#include "encoders.hpp"
#include "ui/led_matrix.hpp"
#include "ui/ui.hpp"
#include "ui/tft_display.hpp"
#include "ui/led_matrix.hpp"
#include "timer.hpp"
#include "keyboard_manager.hpp"
#include <Arduino.h>

struct Engine
{
  AudioGraph audioGraph;
  ModuleBuilder builder;
  KeyboardManager keyboardManager;
};

inline Engine g_Engine;

struct AppContext
{
  enum class State
  {
    Idle,
    Selecting,
    Patching,
    Creating,
    Deleting,
    Displaying
  };

  enum class Mode
  {
    Edit,
    View
  };

  Mode mode{Mode::Edit};
  State state{State::Idle};

  Module* selectedModule{};
  Displayable* selectedDisplayable{};
  Controllable* selectedControllable{};
  Animatable* selectedAnimatable{};

  sndbx::vector_4U<sndbx::event::TrellisPress> trellisEvents{};
};

struct ModuleBankDisplay
{
  sndbx::vector_32U<std::uint32_t> colors;
  std::size_t startIndex{0U};

  ModuleBankDisplay() 
  {
    for (const auto& info : sndbx::engine::bankInfos) 
    {
      colors.push_back(info.color); 
    }
  }
};

inline AppContext g_AppContext;
inline ModuleBankDisplay g_ModuleBankDisplay;
// inline KeyboardManager g_KeyboardManager;

class LEDMatrixManager
{
public:
  LEDMatrixManager(LEDMatrixDisplay& ledMatrix, Engine& engine, AppContext& context) 
  : m_LEDMatrix(ledMatrix), 
    m_Engine(engine),
    m_AppContext(context)
  {}

  void drawConnectionBetween(sndbx::grid::Position srcPos, sndbx::grid::Position destPos)
  {
    const auto moduleDisplay = m_Engine.builder.get<Displayable>(srcPos);

    const std::uint32_t wireColor = moduleDisplay 
      ? sndbx::color::changeBrightness(moduleDisplay->ledColor(), 0.1) 
      : 0x404040;

    auto currentRow = srcPos.row;
    auto currentCol = srcPos.col;

    while (currentRow != destPos.row)
    {
      if (currentRow > destPos.row) { --currentRow; }
      else if (currentRow < destPos.row) { ++currentRow; }
      m_LEDMatrix.drawPixel(currentRow, srcPos.col, wireColor);
    }

    while (currentCol != destPos.col)
    {
      if (currentCol > destPos.col) { --currentCol; }
      else if (currentCol < destPos.col) { ++currentCol; }
      m_LEDMatrix.drawPixel(currentRow, currentCol, wireColor);
    }
  }

  void drawAllConnections()
  {
    m_LEDMatrix.clear();

    for (std::uint8_t i{}; i < m_ModuleDisplays.size(); ++i) 
    {
      const auto position = sndbx::grid::toPosition(i);

      auto module = m_Engine.builder.get<Module>(position);
      if (!module) { continue; }

      for (const auto& port : module->outputs())
      {
        const auto connectedModule = port.connectedModule;
        if (!connectedModule) { continue; }

        if (auto destPos = getPosition(connectedModule))
        {
          drawConnectionBetween(position, *destPos);
        }
      }
    }

    m_Dirty = true;
  }

  void placeModule(Displayable* module, sndbx::grid::Position pos)
  {
    m_ModuleDisplays.at(pos.index()) = module;
    m_LEDMatrix.drawPixel(pos, module->ledColor());
    m_Dirty = false;
  }

  void renderFrame()
  {
    using namespace sndbx; 
    if (m_Dirty) 
    { 
      clear();
      drawAllConnections();
      ui::draw<ModuleBank>(m_LEDMatrix, g_ModuleBankDisplay.colors, g_ModuleBankDisplay.startIndex);
    }

    for (std::size_t i{}; i < m_ModuleDisplays.size(); ++i) 
    { 
      const auto module = m_ModuleDisplays[i];
      if (!module) { continue; }

      auto moduleColor = module->ledColor();
      const auto& selected = m_AppContext.selectedDisplayable;

      if (selected == module)
      {
        moduleColor = color::blend(moduleColor, 0xDDDDFF, 0.1);
      }
      const auto modulePosition = grid::toPosition(static_cast<std::uint8_t>(i));
      m_LEDMatrix.drawPixel(modulePosition, moduleColor);
    }

    m_LEDMatrix.renderFrame();
    m_Dirty = false;
  }

  void clear() { m_LEDMatrix.clear(); }

  void removeModuleDisplay(sndbx::grid::Position pos) 
  { 
    m_ModuleDisplays.at(pos.index()) = nullptr; 
    m_Dirty = true;
  }

  void markDirty() { m_Dirty = true; }

  [[nodiscard]] bool isUpdated() const { return m_Dirty; }
  
private:
  [[nodiscard]] std::optional<sndbx::grid::Position> getPosition(Module* m)  
  {
    for (const auto& entry : m_Engine.builder.registry())
    {
      if (entry.module == m) { return entry.position; }
    }

    return std::nullopt;
  }

private:
  LEDMatrixDisplay& m_LEDMatrix;
  Engine& m_Engine;
  AppContext& m_AppContext;
  bool m_Dirty{true};
  std::array<Displayable*, sndbx::grid::totalCells> m_ModuleDisplays{};
};

class ScreenManager
{
public:
  ScreenManager(TFT& tft, Engine& engine, AppContext& context)
  : m_TFT(tft),
    m_Engine(engine),
    m_AppContext(context)
  {}

  void displaySplash() { sndbx::ui::clearAndDraw<SplashScreen>(m_TFT); }

  void displayModule(Displayable* moduleDisplay, Module* module) 
  {
    sndbx::ui::clearAndDraw<ModuleDisplay>(
      m_TFT,
      moduleDisplay->displayName(),
      moduleDisplay->displayColor(),
      moduleDisplay->controlNames(),
      moduleDisplay->normalizedControlValues(),
      moduleDisplay->inputNames(),
      inputModuleColors(module),
      moduleDisplay->outputNames(),
      outputModuleColors(module)
    );
    m_Dirty = true;
  }

  void displaySelectedModule()
  {
    if (auto animatable = m_AppContext.selectedAnimatable)
    {
      animatable->drawNext(m_TFT.currentFrame());
      m_TFT.setFrameAvailable(true);
      return;
    }

    if (m_Dirty)
    {
      if (auto selected = m_AppContext.selectedDisplayable)
      {
        displayModule(selected, m_AppContext.selectedModule);
      }
    }
  }

  void displayError(sndbx::Error error)
  {
    using namespace sndbx;
    switch (error)
    {
      case Error::BUILDER_INVALID_POS:
        ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't place module here");
        break;
      case Error::BUILDER_POOL_EXHAUSTED:
        ui::clearAndDraw<ErrorDisplay>(m_TFT, 
          "can't create more modules \n         of this type");
        break;
      case Error::BUILDER_REGISTRY_FULL: 
        ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create more modules");
        break;
      default: break;
    }
  }

  void displayError(const sndbx::string32_t& message)
  {
    sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, message.view());
    m_Dirty = true;
  }

  void displayMaxModuleTypeError(const sndbx::engine::ModuleBankEntry& moduleInfo)
  {
    sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create another");
    sndbx::ui::draw<Text>(m_TFT, 0, 70, moduleInfo.name, moduleInfo.color, 1);
    m_Dirty = true;
  }

  void renderFrame() { m_TFT.renderFrame(); }

  void markDirty() { m_Dirty = true; }

  [[nodiscard]] bool isUpdated() const { return m_Dirty; }

private:
  void fillPortColors(sndbx::vector_8U<std::uint32_t>& portColorStorage, const Module::PortArray& ports)
  {
    portColorStorage.clear();
    for (const auto& port : ports)
    {
      if (!port.connectedModule) 
      {
        portColorStorage.push_back(0);
        continue;
      }

      const auto connectedID = port.connectedModule->id();

      if (auto connectedModule = m_Engine.builder.get<Displayable>(connectedID))
      {
        portColorStorage.push_back(connectedModule->displayColor());
      }
    }
  }

  [[nodiscard]] const sndbx::vector_8U<std::uint32_t>& inputModuleColors(Module* parentModule)
  {
    fillPortColors(m_InputModuleColorStorage, parentModule->inputs());
    return m_InputModuleColorStorage;
  }

  [[nodiscard]] const sndbx::vector_8U<std::uint32_t>& outputModuleColors(Module* parentModule)
  {
    fillPortColors(m_OutputModuleColorStorage, parentModule->outputs());
    return m_OutputModuleColorStorage;
  }
  
private:
  TFT& m_TFT;
  Engine& m_Engine;
  AppContext& m_AppContext;
  bool m_Dirty{true};
  sndbx::vector_8U<std::uint32_t> m_InputModuleColorStorage;
  sndbx::vector_8U<std::uint32_t> m_OutputModuleColorStorage;
};

void onEncoderTurn(std::size_t encoderNum, int delta);

struct Input
{
  Encoders encoders;
  Trellis trellis;
};

struct Display
{
  TFT screen;
  LEDMatrixDisplay ledMatrix;
  LEDMatrixManager ledMatrixManager{ledMatrix, g_Engine, g_AppContext};
  ScreenManager screenManager{screen, g_Engine, g_AppContext};

  Display(Trellis& trellis) : ledMatrix(trellis) {}
};

inline Input g_Inputs{onEncoderTurn};
inline Display g_Display{g_Inputs.trellis};


void turnBank(int delta)
{
  const auto newStartIndex = 
      (static_cast<int>(g_ModuleBankDisplay.startIndex) + delta) % 
      static_cast<int>(g_ModuleBankDisplay.colors.size());

  g_ModuleBankDisplay.startIndex = newStartIndex;
  g_Display.ledMatrixManager.markDirty();
}

void onEncoderTurn(std::size_t encoderNum, int delta) 
{
  const auto& buttonPresses = g_AppContext.trellisEvents;

  if (!buttonPresses.is_empty() && sndbx::grid::isBankArea(buttonPresses.at(0).position)) 
  { 
    turnBank(delta);
    return;
  }
  
  if (auto selectedModule = g_AppContext.selectedControllable) 
  { 
    selectedModule->changeControl(encoderNum, delta);
    g_Display.screenManager.markDirty();
    g_Display.ledMatrixManager.markDirty();
  }
}

namespace sndbx::app
{
  void init()
  {
    Serial.begin(115200);
    g_Display.ledMatrixManager.clear();
    g_Display.screenManager.displaySplash();
  }

  bool addKeyboardKey(Keyboard& keyboard) 
  {
    if (auto keyData = g_Engine.keyboardManager.addKey(keyboard, g_Engine.builder))
    {
      g_Display.ledMatrixManager.placeModule(keyData->key, keyData->position);
      return true;
    }
    return false;
  }

  void changeKeyboardScale(Keyboard::Scale scale, std::uint32_t keyboardID) { g_Engine.keyboardManager.changeScale(scale, keyboardID); }

  bool deleteModule(sndbx::grid::Position pos);

  bool subtractKeyboardKey(Keyboard& keyboard) { return g_Engine.keyboardManager.subtractKey(keyboard, deleteModule); }

  void handleDeleteModule(grid::Position pos)
  {
    auto& keyboards = g_Engine.keyboardManager;
    if (keyboards.isKeyboardAt(pos)) { keyboards.deleteKeyboard(pos, deleteModule); }
    if (!keyboards.isKeyAt(pos)) { deleteModule(pos); } //keys only are deleted if parent keyboard is deleted
  }

  void initKeyboard(Keyboard* keyboard, grid::Position pos)
  {
    assert(keyboard);
    
    keyboard->setAddKeyCallback(addKeyboardKey);
    keyboard->setSubtractKeyCallback(subtractKeyboardKey);
    keyboard->setScaleChangeCallback(changeKeyboardScale);
    g_Engine.keyboardManager.addKeyboard(keyboard->id(), pos);
    for (std::size_t keys{}; keys < 8; ++keys) { keyboard->changeControl(0, 1); }
  }

  bool deleteModule(grid::Position pos)
  {
    const auto module = g_Engine.builder.get<Module>(pos);
    if (!module) { return false; }

    patch::disconnectAll(g_Engine.audioGraph, module);

    if (!g_Engine.builder.destroy(pos)) { return false; }
  
    g_Display.ledMatrixManager.removeModuleDisplay(pos);
    return true;
  }

  void createModule(grid::Position bankPos, grid::Position gridPos)
  {
    auto& builder = g_Engine.builder;

    const auto bankIndex = (bankPos.index() - grid::bankStart + g_ModuleBankDisplay.startIndex)
      % g_ModuleBankDisplay.colors.size();

    if (bankIndex == engine::bankIndexOf<Keyboard>()) 
    { 
      const auto result = builder.make<Keyboard>(gridPos);
      if (result) 
      { 
        initKeyboard(result.value, gridPos); 
      }
      else if (result.error == Error::BUILDER_POOL_EXHAUSTED) 
      { 
        g_Display.screenManager.displayMaxModuleTypeError(engine::bankInfos.at(bankIndex)); 
      }
      else 
      {
        g_Display.screenManager.displayError(result.error); 
      }
    }
    else
    {
      const auto error = engine::createModuleFromBankIndex(bankIndex, gridPos, g_Engine.builder);
      if (error == Error::BUILDER_POOL_EXHAUSTED) 
      { 
        g_Display.screenManager.displayMaxModuleTypeError(engine::bankInfos.at(bankIndex)); 
      }
      else { g_Display.screenManager.displayError(error); }
    }
    
    const auto entry = builder.getEntry(gridPos);
    assert(entry);

    g_AppContext.selectedModule       = builder.getFromEntry<Module>(*entry);
    g_AppContext.selectedControllable = builder.getFromEntry<Controllable>(*entry);
    g_AppContext.selectedAnimatable   = builder.getFromEntry<Animatable>(*entry);

    if (const auto moduleDisplay = builder.getFromEntry<Displayable>(*entry))
    {
      g_Display.ledMatrixManager.placeModule(moduleDisplay, gridPos);
      g_AppContext.selectedDisplayable = moduleDisplay;
      g_AppContext.state = AppContext::State::Displaying;
      g_Display.screenManager.markDirty();
    }
  }

  //========================================================================================================================
  // UI
  //========================================================================================================================

  //========================================================================================================================
  // Patching
  //========================================================================================================================

  bool handleDisconnect(Module* src, Module* dest)
  {
    return sndbx::patch::disconnectFirstConnection(g_Engine.audioGraph, src, dest);
  }

  bool handleConnect(Module* src, Displayable* srcDisplay, Module* dest, Displayable* destDisplay)
  {
    const auto output = sndbx::patch::firstAvailablePort(src->outputs());
    const auto input = sndbx::patch::firstAvailablePort(dest->inputs());
    if (!output || !input) { return false; }

    bool connectSuccess = sndbx::patch::connect(g_Engine.audioGraph, src, *output, dest, *input);
    if (!connectSuccess) { return false; }

    if (!srcDisplay || !destDisplay) { return connectSuccess; }

    ui::clearAndDraw<PatchDisplayPage>(
      g_Display.screen,
      srcDisplay->displayName(),
      destDisplay->displayName(),
      srcDisplay->outputNames().at(*output),
      destDisplay->inputNames().at(*input),
      srcDisplay->displayColor(),
      destDisplay->displayColor()
    );

    return true;
  }

  void clearModuleSelections(AppContext& appContext);
  void patchHandler(grid::Position srcPos, grid::Position destPos)
  {
    auto& builder = g_Engine.builder;

    const auto srcEntry = builder.getEntry(srcPos);
    if (!srcEntry) { return; }

    const auto destEntry = builder.getEntry(destPos);
    if (!destEntry) { return; }

    const auto srcModule   = builder.getFromEntry<Module>(*srcEntry);
    const auto destModule  = builder.getFromEntry<Module>(*destEntry);
    const auto srcDisplay  = builder.getFromEntry<Displayable>(*srcEntry);
    const auto destDisplay = builder.getFromEntry<Displayable>(*destEntry);

    const bool patchChanged = 
      sndbx::patch::connectionExists(srcModule, destModule) 
      ? handleDisconnect(srcModule, destModule)
      : handleConnect(srcModule, srcDisplay, destModule, destDisplay);

    if (patchChanged) 
    { 
      clearModuleSelections(g_AppContext);
      Serial.println("setting to patching");
      g_AppContext.state = AppContext::State::Patching;
      g_Display.ledMatrixManager.drawAllConnections();
    }
  }

  //========================================================================================================================
  // Selection dispatch
  //========================================================================================================================

  void clearModuleSelections(AppContext& appContext)
  {
    appContext.selectedModule = nullptr;
    appContext.selectedDisplayable = nullptr;
    appContext.selectedControllable = nullptr;
    appContext.selectedAnimatable = nullptr;
  }

  void handleDoublePress(sndbx::vector_4U<event::TrellisPress>& events)
  {
    const auto& firstEvent = events.at(0);
    const auto& secondEvent = events.at(1);
    const auto& firstPos = firstEvent.position;
    const auto& secondPos = secondEvent.position;

    auto deleteFirstEvent = [](auto& events){ if (!events.is_empty()) events.erase(events.begin()); };

    if (firstPos == secondPos) 
    { 
      deleteFirstEvent(events); 
      return;
    }

    if (grid::isBuildableArea(firstPos) && grid::isBuildableArea(secondPos))
    {
      constexpr static auto patchActionTimeout = 1500u;
      if (secondEvent.time - firstEvent.time < patchActionTimeout) 
      {
        patchHandler(firstPos, secondPos);
        events.clear();
      }
      else { deleteFirstEvent(events); }
    }
    else if (grid::isBankArea(firstPos) && grid::isBuildableArea(secondPos))
    {
      g_AppContext.state = AppContext::State::Creating;
      createModule(firstPos, secondPos);
      events.clear();
    }
    else { deleteFirstEvent(events); }
  }

  void handleGridPress(const event::TrellisPress& event, sndbx::vector_4U<event::TrellisPress>& events)
  {
    const auto& builder = g_Engine.builder;
    const auto& position = event.position;

    if (events.size() == 2) { return handleDoublePress(events); }

    const auto entry = builder.getEntry(position);
    if (!entry) { return; }

    clearModuleSelections(g_AppContext);

    g_AppContext.selectedModule = builder.getFromEntry<Module>(*entry);
    g_AppContext.selectedControllable = builder.getFromEntry<Controllable>(*entry); //no check needed, if module isn't controllable then selection set to null
    
    if (auto pressable = builder.getFromEntry<Pressable>(*entry)) 
    { 
      pressable->onRisingEdge(); 
      g_Display.ledMatrixManager.markDirty();
    }

    if (const auto displayable = builder.getFromEntry<Displayable>(*entry))
    {
      g_AppContext.selectedDisplayable = displayable;
      g_AppContext.state = AppContext::State::Displaying;
      g_Display.screenManager.markDirty();
    }

    if (const auto animatable = builder.getFromEntry<Animatable>(*entry))
    {
      g_AppContext.selectedAnimatable = animatable;
      g_AppContext.state = AppContext::State::Displaying;
    }
  }

  void handleTrellisRisingEdge(const event::TrellisPress& event)
  {
    g_AppContext.trellisEvents.push_back(event);
    handleGridPress(event, g_AppContext.trellisEvents);
  }

  void handleTrellisFallingEdge(const event::TrellisPress& event)
  {
    if (auto pressableModule = g_Engine.builder.get<Pressable>(event.position))
    {
      pressableModule->onFallingEdge();
      g_Display.ledMatrixManager.markDirty();
    }
  }

  void handleLongPress(const event::TrellisPress& event)
  {
    switch (g_AppContext.mode)
    {
      case AppContext::Mode::Edit: 
        handleDeleteModule(event.position); 
        clearModuleSelections(g_AppContext);
        break;
      case AppContext::Mode::View: 
        break;
    }
  }

  void handleTrellisPress(const event::TrellisPress& event)
  {
    static Timer holdTimer;
    constexpr static auto holdThresholdMs = 1000u;

    const auto edge = event.edge;

    if (edge == event::Edge::RISING_EDGE)
    {
      holdTimer.start();
      handleTrellisRisingEdge(event); 
    }
    else if (edge == event::Edge::FALLING_EDGE) 
    { 
      if (holdTimer.hasReached(holdThresholdMs)) { handleLongPress(event); }
      handleTrellisFallingEdge(event);
    }
  }

  float processorUsage()
  {
    float total{};
    for (const auto& entry : g_Engine.builder.registry()) 
    { 
      total += entry.module->audio().processorUsage(); 
    }
    return total;
  }

  void readInputs()
  {
    g_Inputs.trellis.update();
    g_Inputs.encoders.update();

    if (g_Inputs.trellis.hasEvent()) 
    {
      handleTrellisPress(*g_Inputs.trellis.popEvent()); 
    }
  }

  void updateAppState() 
  {
    switch (g_AppContext.state)
    {
      case AppContext::State::Idle:
      case AppContext::State::Patching: break;
      case AppContext::State::Creating:
      case AppContext::State::Deleting:
      case AppContext::State::Displaying:
        g_Display.screenManager.displaySelectedModule();
        break;
      case AppContext::State::Selecting:
      default: break;
    }
  }

  void renderDisplay()
  {
    g_Display.ledMatrixManager.renderFrame();
    g_Display.screenManager.renderFrame();
  }

  void loop() 
  {
    readInputs();
    updateAppState();
    renderDisplay();
  }
}