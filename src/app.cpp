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
#include <vector>
#include <map>

namespace sndbx::engine
{
  inline AudioGraph audioGraph;
  inline ModuleBuilder builder;
}

struct AppContext
{
  enum struct State
  {
    Idle,
    Selecting,
    Patching,
    Creating,
    Deleting,
    Displaying
  };

  enum struct Mode
  {
    Edit,
    View
  };


  Mode currentMode{Mode::Edit};
  State currentState{State::Idle};

  std::optional<Module*> selectedModule{};
  std::optional<Displayable*> selectedDisplayable{};
  std::optional<Controllable*> selectedControllable{};
  std::optional<Animatable*> selectedAnimatable{};

  std::vector<sndbx::event::TrellisPress> trellisEvents{};
};

struct DisplayContext
{
  std::map<sndbx::grid::Position, Displayable*> moduleDisplays{};
  Displayable* lastDisplayed{};
  bool ledsUpdated{false};
  bool displayUpdated{false};
};

struct ModuleBankDisplay
{
  std::vector<std::uint32_t> colors;
  std::size_t currentOffset{0U};

  ModuleBankDisplay() 
  {
    colors.reserve(ModuleTypes::size);
    for (auto info : bankInfos) { colors.push_back(info.color); }
  }
};

inline AppContext g_AppContext;
inline DisplayContext g_DisplayContext;
inline ModuleBankDisplay g_ModuleBankDisplay;

namespace sndbx::input
{
  void onTurn(std::size_t encoderNum, int delta);
  inline Encoders encoders(onTurn);
  inline Trellis trellis;
}

namespace sndbx::display
{
  inline TFT screen;
  inline LEDMatrixDisplay ledMatrix(input::trellis);
}

namespace sndbx::input
{
  void turnBank(int delta)
  {
    if (delta < 0 && g_ModuleBankDisplay.currentOffset == 0) { return; }

    const auto newOffset = g_ModuleBankDisplay.currentOffset + delta;
    if (newOffset >= g_ModuleBankDisplay.colors.size() - grid::cols) { return; }

    ui::draw<ModuleBank>(display::ledMatrix, g_ModuleBankDisplay.colors, newOffset);
    g_ModuleBankDisplay.currentOffset = newOffset;
    ::g_DisplayContext.ledsUpdated = false;
  }

  void onTurn(std::size_t encoderNum, int delta) 
  {
    const auto& buttonPresses = ::g_AppContext.trellisEvents;

    if (!buttonPresses.empty()) 
    { 
      if (grid::isBankArea(buttonPresses.at(0).position)) 
      { 
        turnBank(delta);
        ::g_DisplayContext.ledsUpdated = false;
        return;
      }
    }
    
    auto& selectedModule = ::g_AppContext.selectedControllable;
    if (selectedModule) 
    { 
      selectedModule.value()->changeControl(encoderNum, delta);
      ::g_DisplayContext.displayUpdated = false;
      ::g_DisplayContext.ledsUpdated = false;
    }
  }
}

namespace sndbx::app
{
  void init()
  {
    Serial.begin(115200);
    ui::clear(display::ledMatrix);
    ui::clearAndDraw<SplashScreen>(display::screen);
  }

  void placeModule(Displayable* module, grid::Position pos)
  {
    ::g_DisplayContext.moduleDisplays[pos] = module;
    display::ledMatrix.drawPixel(pos, module->ledColor());
    ::g_DisplayContext.ledsUpdated = false;
  }

  [[nodiscard]] std::vector<std::uint32_t> connectedModuleColors(const std::vector<Module::Port>& ports, ModuleBuilder& builder)
  {
    std::vector<std::uint32_t> portColors;
    portColors.reserve(ports.size());

    for (const auto& port : ports)
    {
      if (!port.m_Connected) 
      {
        portColors.push_back(0);
        continue;
      }

      const auto connectedID = port.m_Connected->id();

      if (Displayable* connectedModule = builder.get<Displayable*>(connectedID))
      {
        portColors.push_back(connectedModule->displayColor());
      }
    }

    return portColors;
  }
  
  void displayModule(Displayable* moduleDisplay, Module* module) 
  {
    ui::clearAndDraw<ModuleDisplay>(
      display::screen,
      moduleDisplay->displayName(),
      moduleDisplay->displayColor(),
      moduleDisplay->controlNames(),
      moduleDisplay->normalizedControlValues(),
      moduleDisplay->inputNames(),
      connectedModuleColors(module->inputs(), engine::builder),
      moduleDisplay->outputNames(),
      connectedModuleColors(module->outputs(), engine::builder)
    );
  }
  
  std::map<std::uint32_t, sndbx::grid::Position> keyboardStartPositions;
  std::map<std::uint32_t, sndbx::grid::Position> keyboardEndPositions;

  bool addKeyboardKey(Keyboard& keyboard)
  {
    const auto keyboardID = keyboard.id();

    auto it = keyboardEndPositions.find(keyboardID);
    if (it == keyboardEndPositions.end()) { return false; }
  
    const auto lastKeyPos = (*it).second.index();
    const auto newKeyPos = sndbx::grid::toPosition(lastKeyPos + 1);

    if (!grid::isBuildableArea(newKeyPos)) { return false; }

    auto key = engine::builder.make<KeyboardKey>(newKeyPos, keyboard);
    if (!key) { return false; }
    
    keyboardEndPositions[keyboardID] = newKeyPos;
    return true;
  }

  bool deleteModule(const sndbx::grid::Position& pos);
  bool subtractKeyboardKey(Keyboard& keyboard)
  {
    auto it = keyboardEndPositions.find(keyboard.id());
    if (it == keyboardEndPositions.end()) { return false; }

    const auto lastKeyPos = (*it).second;
    keyboardEndPositions.erase(it);
    
    return deleteModule(lastKeyPos);
  }

  void clearModuleSelections(AppContext& appContext);
  bool deleteModule(const grid::Position& pos)
  {
    auto module = engine::builder.get<Module*>(pos);
    if (!module) { return false; }

    engine::audioGraph.deletePatchesWith(module);

    if (!engine::builder.destroy(pos)) { return false; }
  
    auto& moduleDisplays = ::g_DisplayContext.moduleDisplays;
    if (moduleDisplays.find(pos) != moduleDisplays.end()) 
    { 
      moduleDisplays.erase(pos); 
    }
    clearModuleSelections(g_AppContext);
    ::g_DisplayContext.ledsUpdated = false;
    
    return true;
  }

  void createModule(const grid::Position bankPos, const grid::Position gridPos)
  {
    const std::size_t typeIndex = bankPos.index() - grid::bankStart + g_ModuleBankDisplay.currentOffset;

    if (typeIndex == engine::typeIndexOf<Keyboard>())
    {
      auto keyboard = engine::builder.make<Keyboard>(gridPos);
      if (keyboard) 
      {
        keyboard->setAddKeyCallback(addKeyboardKey);
        keyboard->setSubtractKeyCallback(subtractKeyboardKey);
        keyboardStartPositions[keyboard->id()] = gridPos;
        keyboardEndPositions[keyboard->id()] = gridPos;
      }
    }
    else
    {
      engine::buildFromTypeIndex(typeIndex, gridPos, engine::builder);
    }

    if (auto displayable = engine::builder.get<Displayable*>(gridPos))
    {
      placeModule(displayable, gridPos);
    }
  }

  //========================================================================================================================
  // UI
  //========================================================================================================================
  void drawConnectionBetween(LEDMatrixDisplay& ledMatrix, grid::Position srcPos, grid::Position destPos)
  {
    auto moduleDisplay = engine::builder.get<Displayable*>(srcPos);
    std::uint32_t wireColor = moduleDisplay ? sndbx::color::changeBrightness(moduleDisplay->ledColor(), 0.1) : 0x404040;

    auto currentRow = srcPos.row;
    auto currentCol = srcPos.col;

    while (currentRow != destPos.row)
    {
      if (currentRow > destPos.row) { currentRow--; }
      else if (currentRow < destPos.row) { currentRow++; }
      ledMatrix.drawPixel(currentRow, srcPos.col, wireColor);
    }

    while (currentCol != destPos.col)
    {
      if (currentCol > destPos.col) { currentCol--; }
      else if (currentCol < destPos.col) { currentCol++; }
      ledMatrix.drawPixel(currentRow, currentCol, wireColor);
    }
  }

  void drawAllConnections(LEDMatrixDisplay& ledMatrix)
  {
    ui::clear(ledMatrix);
    for (auto& [position, _] : ::g_DisplayContext.moduleDisplays) 
    { 
      auto module = engine::builder.get<Module*>(position);
      if (!module) { continue; }

      for (const auto& port : module->outputs())
      {
        const auto connectedModule = port.m_Connected;
        if (!connectedModule) { continue; }

        if (auto destPos = engine::builder.getPosition(connectedModule))
        {
          drawConnectionBetween(ledMatrix, position, *destPos);
        }
      }
    }
    ::g_DisplayContext.ledsUpdated = false;
  }

  //========================================================================================================================
  // Patching
  //========================================================================================================================

  bool handleDisconnect(Module* src, Module* dest)
  {
    return sndbx::patch::disconnectFirstConnection(engine::audioGraph, src, dest);
  }

  bool handleConnect(Module* src, grid::Position srcPos, Module* dest, grid::Position destPos)
  {
    const auto output = sndbx::patch::firstAvailablePort(src->outputs());
    const auto input = sndbx::patch::firstAvailablePort(dest->inputs());
    if (!output || !input) { return false; }

    bool success = sndbx::patch::connect(engine::audioGraph, src, *output, dest, *input);
    if (!success) { return false; }

    auto srcDisplay = engine::builder.get<Displayable*>(srcPos);
    auto destDisplay = engine::builder.get<Displayable*>(destPos);
    if (!srcDisplay || !destDisplay) { return success; }

    ui::clearAndDraw<PatchDisplayPage>(
      display::screen,
      srcDisplay->displayName(),
      destDisplay->displayName(),
      srcDisplay->outputNames().at(*output),
      destDisplay->inputNames().at(*input),
      srcDisplay->displayColor(),
      destDisplay->displayColor()
    );

    return true;
  }

  void patchHandler(grid::Position srcPos, grid::Position destPos)
  {
    auto* srcModule = engine::builder.get<Module*>(srcPos);
    if (!srcModule) { return; }

    auto* destModule = engine::builder.get<Module*>(destPos);
    if (!destModule) { return; }

    bool success = 
      sndbx::patch::connectionExists(srcModule, destModule) 
      ? handleDisconnect(srcModule, destModule)
      : handleConnect(srcModule, srcPos, destModule, destPos);

    if (success) 
    { 
      clearModuleSelections(::g_AppContext);
      ::g_AppContext.currentState = AppContext::State::Patching;
      drawAllConnections(display::ledMatrix); 
      ::g_DisplayContext.displayUpdated = false;
    }
  }

  //========================================================================================================================
  // Selection dispatch
  //========================================================================================================================

  void clearModuleSelections(AppContext& appContext)
  {
    appContext.selectedModule.reset();
    appContext.selectedDisplayable.reset();
    appContext.selectedControllable.reset();
  }

  void handleDoublePress(std::vector<event::TrellisPress>& events)
  {
    const auto& firstEvent = events.at(0);
    const auto& secondEvent = events.at(1);
    const auto& firstPos = firstEvent.position;
    const auto& secondPos = secondEvent.position;

    auto clearFirst = [](auto& events){ if (!events.empty()) events.erase(events.begin()); };

    if (firstPos == secondPos) 
    { 
      clearFirst(events); 
      return;
    }

    if (grid::isBuildableArea(firstPos) && grid::isBuildableArea(secondPos))
    {
      constexpr static auto maxTimeBetweenSelections = 1500ul;
      if (secondEvent.time - firstEvent.time > maxTimeBetweenSelections) 
      {
        clearFirst(events);
        return;
      }
      patchHandler(firstPos, secondPos);
      events.clear();
    }
    else if (grid::isBankArea(firstPos) && grid::isBuildableArea(secondPos))
    {
      ::g_AppContext.currentState = AppContext::State::Creating;
      createModule(firstPos, secondPos);
      events.clear();
    }
    else
    {
      clearFirst(events);
    }
  }

  void handleGridPress(const event::TrellisPress& event, std::vector<event::TrellisPress>& events)
  {
    auto& builder = engine::builder;
    auto& position = event.position;

    if (events.size() == 2) { handleDoublePress(events); }

    if (auto module = builder.get<Module*>(position)) 
    { 
      ::g_AppContext.selectedModule = module;
    } 
    else { return; } //not occupied

    if (auto pressable = builder.get<Pressable*>(position)) 
    { 
      pressable->onRisingEdge(); 
      ::g_DisplayContext.ledsUpdated = false;
    }

    if (auto controllable = builder.get<Controllable*>(position))
    {
      ::g_AppContext.selectedControllable = controllable;
    }

    if (auto displayable = builder.get<Displayable*>(position))
    {
      ::g_AppContext.selectedDisplayable = displayable;
      ::g_AppContext.currentState = AppContext::State::Displaying;
    }

    if (auto animatable = builder.get<Animatable*>(position))
    {
      ::g_AppContext.selectedAnimatable = animatable;
    }
  }

  void handleTrellisRisingEdge(const event::TrellisPress& event)
  {
    ::g_AppContext.trellisEvents.push_back(event);

      Serial.print("{ ");
      for (auto p : ::g_AppContext.trellisEvents)
      {
        Serial.printf("%d, ", p.position.index());
      }
      Serial.print("}\n");

    handleGridPress(event, ::g_AppContext.trellisEvents);
  }

  void handleTrellisFallingEdge(const event::TrellisPress& event)
  {
    if (auto pressableModule = engine::builder.get<Pressable*>(event.position))
    {
      pressableModule->onFallingEdge();
      ::g_DisplayContext.ledsUpdated = false;
    }
  }

  void handleLongPress(const event::TrellisPress& event)
  {
    switch (::g_AppContext.currentMode)
    {
      case AppContext::Mode::Edit: 
        deleteModule(event.position); 
        break;
      case AppContext::Mode::View: 
        break;
    }
  }

  void handleTrellisPress(const event::TrellisPress& event)
  {
    static Timer holdTimer;
    constexpr static auto holdThresholdMs = 2000;

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

  void updateLEDs(LEDMatrixDisplay& ledMatrix)
  {
    for (auto& [position, module] : ::g_DisplayContext.moduleDisplays) 
    { 
      ledMatrix.drawPixel(position, module->ledColor());
    }
    
    if (!::g_DisplayContext.ledsUpdated) 
    { 
      ui::draw<ModuleBank>(ledMatrix, g_ModuleBankDisplay.colors, g_ModuleBankDisplay.currentOffset);
    }

    ledMatrix.renderFrame();
    ::g_DisplayContext.ledsUpdated = true;
  }

  void displaySelectedModule()
  {
    if (auto selected = ::g_AppContext.selectedDisplayable)
    {
      Displayable* cur = *selected;
      if (cur != ::g_DisplayContext.lastDisplayed || !::g_DisplayContext.displayUpdated)
      {
        displayModule(cur, *(::g_AppContext.selectedModule));
        ::g_DisplayContext.lastDisplayed = cur;
        ::g_DisplayContext.displayUpdated = true;
      }
    }
    if (auto animatable = ::g_AppContext.selectedAnimatable)
    {
      (*animatable)->drawNext(display::screen.currentFrame());
    }
  }

  float cpuUsage()
  {
    float total{};
    for (const auto& e : engine::builder.m_ModuleRegistry) { total += e.module.get()->audio().cpuUsage(); }
    return total;
  }

  void readInputs()
  {
    input::trellis.update();
    input::encoders.update();

    if (input::trellis.hasEvent()) 
    {
      Serial.println(cpuUsage());
      handleTrellisPress(*input::trellis.popEvent()); 
    }
  }

  void updateState() 
  {
    switch (::g_AppContext.currentState)
    {
      case AppContext::State::Idle:
      case AppContext::State::Creating:
      case AppContext::State::Deleting:
      case AppContext::State::Displaying:
        displaySelectedModule();
        break;
      case AppContext::State::Patching:
      case AppContext::State::Selecting:
      default: break;
    }
  }

  void renderDisplay()
  {
    updateLEDs(display::ledMatrix);
    display::screen.renderFrame();
  }

  void loop() 
  {
    readInputs();
    updateState();
    renderDisplay();
  }
}