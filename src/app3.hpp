// #include "engine/module_builder.hpp"
// #include "engine/patching.hpp"
// #include "engine/builder_interface.hpp"
// #include "modules/dep/module.hpp"
// #include "modules/dep/module_interfaces.hpp"
// #include "trellis.hpp"
// #include "encoders.hpp"
// #include "ui/led_matrix.hpp"
// #include "ui/ui.hpp"
// #include "ui/tft_display.hpp"
// #include "ui/led_matrix.hpp"
// #include "timer.hpp"
// #include "keyboard_manager.hpp"
// #include <vector>
// #include <map>
// #include <new>
// #include <Arduino.h>


// struct ModuleBankDisplay
// {
//   sndbx::vector_32U<std::uint32_t> colors;
//   std::size_t startIndex{0U};

//   ModuleBankDisplay() 
//   {
//     for (const auto& info : sndbx::engine::bankInfos) 
//     {
//       colors.push_back(info.color); 
//     }
//   }
// };

// inline ModuleBankDisplay g_ModuleBankDisplay;

// struct AppContext
// {
//   enum class State
//   {
//     Idle,
//     Selecting,
//     Patching,
//     Creating,
//     Deleting,
//     Displaying
//   };

//   enum class Mode { Edit, View };

//   Mode mode{Mode::Edit};
//   State state{State::Idle};

//   Module* selectedModule{};
//   Displayable* selectedDisplayable{};
//   Controllable* selectedControllable{};
//   Animatable* selectedAnimatable{};

//   sndbx::vector_4U<sndbx::event::TrellisPress> trellisEvents{};
// };

// class LEDMatrixManager
// {
// public:
//   LEDMatrixManager(LEDMatrixDisplay& ledMatrix) : m_LEDMatrix(ledMatrix) {}

//   void drawConnectionBetween(sndbx::grid::Position srcPos, sndbx::grid::Position destPos, const ModuleBuilder& builder)
//   {
//     const auto moduleDisplay = builder.get<Displayable>(srcPos);

//     const std::uint32_t wireColor = 
//       moduleDisplay 
//       ? sndbx::color::changeBrightness(moduleDisplay->ledColor(), 0.1) 
//       : 0x404040;

//     auto currentRow = srcPos.row;
//     auto currentCol = srcPos.col;

//     while (currentRow != destPos.row)
//     {
//       if (currentRow > destPos.row) { --currentRow; }
//       else if (currentRow < destPos.row) { ++currentRow; }
//       m_LEDMatrix.drawPixel(currentRow, srcPos.col, wireColor);
//     }

//     while (currentCol != destPos.col)
//     {
//       if (currentCol > destPos.col) { --currentCol; }
//       else if (currentCol < destPos.col) { ++currentCol; }
//       m_LEDMatrix.drawPixel(currentRow, currentCol, wireColor);
//     }
//   }

//   void drawAllConnections(const ModuleBuilder& builder)
//   {
//     m_LEDMatrix.clear();

//     for (std::size_t i{}; i < m_ModuleDisplays.size(); ++i) 
//     {
//       const auto position = sndbx::grid::toPosition(i);

//       const auto module = builder.get<Module>(position);
//       if (!module) { continue; }

//       for (const auto& port : module->outputs())
//       {
//         const auto connectedModule = port.connectedModule;
//         if (!connectedModule) { continue; }

//         if (auto destPos = getPosition(connectedModule, builder))
//         {
//           drawConnectionBetween(position, *destPos, builder);
//         }
//       }
//     }

//     m_Dirty = true;
//   }

//   void placeModule(Displayable* module, sndbx::grid::Position pos)
//   {
//     m_ModuleDisplays.at(pos.index()) = module;
//     m_LEDMatrix.drawPixel(pos, module->ledColor());
//     m_Dirty = false;
//   }

//   void renderFrame(const AppContext& context, const ModuleBuilder& builder)
//   {
//     using namespace sndbx; 
//     if (m_Dirty) 
//     { 
//       clear();
//       drawAllConnections(builder);
//       ui::draw<ModuleBank>(m_LEDMatrix, g_ModuleBankDisplay.colors, g_ModuleBankDisplay.startIndex);
//     }

//     for (std::size_t i{}; i < m_ModuleDisplays.size(); ++i) 
//     { 
//       const auto module = m_ModuleDisplays[i];
//       if (!module) { continue; }

//       auto moduleColor = module->ledColor();
//       const auto& selected = context.selectedDisplayable;

//       if (selected == module)
//       {
//         moduleColor = color::blend(moduleColor, 0xDDDDFF, 0.1);
//       }
//       const auto modulePosition = grid::toPosition(i);
//       m_LEDMatrix.drawPixel(modulePosition, moduleColor);
//     }

//     m_LEDMatrix.renderFrame();
//     m_Dirty = false;
//   }

//   void clear() { m_LEDMatrix.clear(); }

//   void removeModuleDisplay(sndbx::grid::Position pos) 
//   { 
//     m_ModuleDisplays.at(pos.index()) = nullptr; 
//     m_Dirty = true;
//   }

//   void markDirty() { m_Dirty = true; }

//   [[nodiscard]] bool isUpdated() const { return m_Dirty; }
  
// private:
//   [[nodiscard]] std::optional<sndbx::grid::Position> getPosition(Module* m, const ModuleBuilder& builder)  
//   {
//     for (const auto& entry : builder.registry())
//     {
//       if (entry.module == m) { return entry.position; }
//     }

//     return std::nullopt;
//   }

// private:
//   LEDMatrixDisplay& m_LEDMatrix;
//   bool m_Dirty{true};
//   std::array<Displayable*, sndbx::grid::totalCells> m_ModuleDisplays{};
// };

// class ScreenManager
// {
// public:
//   ScreenManager(TFT& tft) : m_TFT(tft) {}

//   void displaySplash() { sndbx::ui::clearAndDraw<SplashScreen>(m_TFT); }

//   void displayModule(Displayable* moduleDisplay, Module* module, const ModuleBuilder& builder) 
//   {
//     sndbx::ui::clearAndDraw<ModuleDisplay>(
//       m_TFT,
//       moduleDisplay->displayName(),
//       moduleDisplay->displayColor(),
//       moduleDisplay->controlNames(),
//       moduleDisplay->normalizedControlValues(),
//       moduleDisplay->inputNames(),
//       inputModuleColors(module, builder),
//       moduleDisplay->outputNames(),
//       outputModuleColors(module, builder)
//     );
//     m_Dirty = true;
//   }

//   void displaySelectedModule(const AppContext& context, const ModuleBuilder& builder)
//   {
//     if (auto animatable = context.selectedAnimatable)
//     {
//       animatable->drawNext(m_TFT.currentFrame());
//       m_TFT.setFrameAvailable(true);
//       return;
//     }

//     if (m_Dirty)
//     {
//       if (auto selected = context.selectedDisplayable)
//       {
//         displayModule(selected, context.selectedModule, builder);
//       }
//     }
//   }

//   void displayError(sndbx::Error error)
//   {
//     using namespace sndbx;
//     switch (error)
//     {
//       case Error::BUILDER_INVALID_POS:
//         ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't place module here");
//         break;
//       case Error::BUILDER_POOL_EXHAUSTED:
//         ui::clearAndDraw<ErrorDisplay>(m_TFT, 
//           "can't create more modules \n         of this type");
//         break;
//       case Error::BUILDER_REGISTRY_FULL: 
//         ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create more modules");
//         break;
//       default: break;
//     }
//   }

//   void displayError(const sndbx::string32_t& message)
//   {
//     sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, message.view());
//     m_Dirty = true;
//   }

//   void displayMaxModuleTypeError(const sndbx::engine::ModuleBankEntry& moduleInfo)
//   {
//     sndbx::ui::clearAndDraw<ErrorDisplay>(m_TFT, "can't create another");
//     sndbx::ui::draw<Text>(m_TFT, 0, 70, moduleInfo.name, moduleInfo.color, 1);
//     m_Dirty = true;
//   }

//   void markDirty() { m_Dirty = true; }

//   [[nodiscard]] bool isUpdated() const { return m_Dirty; }

// private:
//   void fillPortColors(sndbx::vector_8U<std::uint32_t>& portColorStorage, const Module::PortArray& ports, const ModuleBuilder& builder)
//   {
//     portColorStorage.clear();
//     for (const auto& port : ports)
//     {
//       if (!port.connectedModule) 
//       {
//         portColorStorage.push_back(0);
//         continue;
//       }

//       const auto connectedID = port.connectedModule->id();

//       if (auto connectedModule = builder.get<Displayable>(connectedID))
//       {
//         portColorStorage.push_back(connectedModule->displayColor());
//       }
//     }
//   }

//   [[nodiscard]] const sndbx::vector_8U<std::uint32_t>& inputModuleColors(Module* parentModule, const ModuleBuilder& builder)
//   {
//     fillPortColors(m_InputModuleColorStorage, parentModule->inputs(), builder);
//     return m_InputModuleColorStorage;
//   }

//   [[nodiscard]] const sndbx::vector_8U<std::uint32_t>& outputModuleColors(Module* parentModule, const ModuleBuilder& builder)
//   {
//     fillPortColors(m_OutputModuleColorStorage, parentModule->outputs(), builder);
//     return m_OutputModuleColorStorage;
//   }
  
// private:
//   TFT& m_TFT;
//   bool m_Dirty{true};
//   sndbx::vector_8U<std::uint32_t> m_InputModuleColorStorage;
//   sndbx::vector_8U<std::uint32_t> m_OutputModuleColorStorage;
// };

// class App
// {
// public:
//   App()
//   {
//     app = this;
//     Serial.begin(115200);
//     m_LEDMatrixManager.clear();
//     m_ScreenManager.displaySplash();
//   }

// private:
//   void turnBank(int delta)
//   {
//     const auto newStartIndex = 
//         (static_cast<int>(g_ModuleBankDisplay.startIndex) + delta) % 
//         static_cast<int>(g_ModuleBankDisplay.colors.size());

//     g_ModuleBankDisplay.startIndex = newStartIndex;
//     m_LEDMatrixManager.markDirty();
//   }

//   static void onEncoderTurn(std::size_t encoderNum, int delta)
//   {
//     const auto& buttonPresses = app->m_Context.trellisEvents;

//     if (!buttonPresses.is_empty() && sndbx::grid::isBankArea(buttonPresses.at(0).position)) 
//     { 
//       app->turnBank(delta);
//       return;
//     }
    
//     if (auto selectedModule = app->m_Context.selectedControllable) 
//     { 
//       selectedModule->changeControl(encoderNum, delta);
//       app->m_ScreenManager.markDirty();
//       app->m_LEDMatrixManager.markDirty();
//     }
//   }

//   static bool addKeyboardKey(Keyboard& keyboard) 
//   {
//     if (auto keyData = KeyboardManager::addKey(keyboard, app->m_ModuleBuilder))
//     {
//       app->m_LEDMatrixManager.placeModule(keyData->key, keyData->position);
//       return true;
//     }
//     return false;
//   }

// public:
//   static bool deleteModule(sndbx::grid::Position pos)
//   {
//     const auto module = app->m_ModuleBuilder.get<Module>(pos);
//     if (!module) { return false; }

//     sndbx::patch::disconnectAll(app->m_AudioGraph, module);

//     if (!app->m_ModuleBuilder.destroy(pos)) { return false; }
  
//     app->m_LEDMatrixManager.removeModuleDisplay(pos);
//     return true;
//   }

//   static bool subtractKeyboardKey(Keyboard& keyboard) { return KeyboardManager::subtractKey(keyboard, deleteModule); }

//   void handleDeleteModule(sndbx::grid::Position pos)
//   {
//     if (KeyboardManager::isKeyboardAt(pos)) { KeyboardManager::deleteKeyboard(pos, deleteModule); }
//     if (KeyboardManager::isKeyAt(pos)) { deleteModule(pos); }
//   }

//   void initKeyboard(Keyboard* keyboard, sndbx::grid::Position pos)
//   {
//     assert(keyboard);
    
//     keyboard->setAddKeyCallback(addKeyboardKey);
//     keyboard->setSubtractKeyCallback(subtractKeyboardKey);
//     keyboard->setScaleChangeCallback(KeyboardManager::changeScale);
//     KeyboardManager::addKeyboard(keyboard->id(), pos);
//   }

//   void createModule(sndbx::grid::Position bankPos, sndbx::grid::Position gridPos)
//   {
//     const auto bankIndex = (bankPos.index() - sndbx::grid::bankStart + g_ModuleBankDisplay.startIndex)
//       % g_ModuleBankDisplay.colors.size();

//     //Serial.println(bankIndex);
//     if (bankIndex == sndbx::engine::bankIndexOf<Keyboard>()) 
//     { 
//       const auto result = m_ModuleBuilder.make<Keyboard>(gridPos);
//       if (result) 
//       { 
//         initKeyboard(result.value, gridPos); 
//       }
//       else if (result.error == sndbx::Error::BUILDER_POOL_EXHAUSTED) 
//       { 
//         m_ScreenManager.displayMaxModuleTypeError(sndbx::engine::bankInfos.at(bankIndex)); 
//       }
//       else 
//       {
//         m_ScreenManager.displayError(result.error); 
//       }
//     }
//     else
//     {
//       const auto error = sndbx::engine::createModuleFromBankIndex(bankIndex, gridPos, m_ModuleBuilder);
//       if (error == sndbx::Error::BUILDER_POOL_EXHAUSTED) 
//       { 
//         m_ScreenManager.displayMaxModuleTypeError(sndbx::engine::bankInfos.at(bankIndex)); 
//       }
//       else { m_ScreenManager.displayError(error); }
//     }
    
//     const auto entry = m_ModuleBuilder.getEntry(gridPos);
//     assert(entry);

//     m_Context.selectedModule       = m_ModuleBuilder.getFromEntry<Module>(*entry);
//     m_Context.selectedControllable = m_ModuleBuilder.getFromEntry<Controllable>(*entry);
//     m_Context.selectedAnimatable   = m_ModuleBuilder.getFromEntry<Animatable>(*entry);

//     if (const auto moduleDisplay = m_ModuleBuilder.getFromEntry<Displayable>(*entry))
//     {
//       m_Context.selectedDisplayable = moduleDisplay;
//       m_Context.state = AppContext::State::Displaying;
//       m_LEDMatrixManager.placeModule(moduleDisplay, gridPos);
//       m_LEDMatrixManager.markDirty();
//     }
//   }

//   bool handleDisconnect(Module* src, Module* dest)
//   {
//     return sndbx::patch::disconnectFirstConnection(m_AudioGraph, src, dest);
//   }

//   bool handleConnect(Module* src, Displayable* srcDisplay, Module* dest, Displayable* destDisplay)
//   {
//     const auto output = sndbx::patch::firstAvailablePort(src->outputs());
//     const auto input = sndbx::patch::firstAvailablePort(dest->inputs());
//     if (!output || !input) { return false; }

//     bool connectSuccess = sndbx::patch::connect(m_AudioGraph, src, *output, dest, *input);
//     if (!connectSuccess) { return false; }

//     if (!srcDisplay || !destDisplay) { return connectSuccess; }

//     sndbx::ui::clearAndDraw<PatchDisplayPage>(
//       m_Screen,
//       srcDisplay->displayName(),
//       destDisplay->displayName(),
//       srcDisplay->outputNames().at(*output),
//       destDisplay->inputNames().at(*input),
//       srcDisplay->displayColor(),
//       destDisplay->displayColor()
//     );

//     return true;
//   }

//   void patchHandler(sndbx::grid::Position srcPos, sndbx::grid::Position destPos)
//   {
//     const auto srcEntry = m_ModuleBuilder.getEntry(srcPos);
//     if (!srcEntry) { return; }

//     const auto destEntry = m_ModuleBuilder.getEntry(destPos);
//     if (!destEntry) { return; }

//     const auto srcModule   = m_ModuleBuilder.getFromEntry<Module>(*srcEntry);
//     const auto destModule  = m_ModuleBuilder.getFromEntry<Module>(*destEntry);
//     const auto srcDisplay  = m_ModuleBuilder.getFromEntry<Displayable>(*srcEntry);
//     const auto destDisplay = m_ModuleBuilder.getFromEntry<Displayable>(*destEntry);

//     const bool patchChanged = 
//       sndbx::patch::connectionExists(srcModule, destModule) 
//       ? handleDisconnect(srcModule, destModule)
//       : handleConnect(srcModule, srcDisplay, destModule, destDisplay);

//     if (patchChanged) 
//     { 
//       clearModuleSelections();
//       Serial.println("setting to patching");
//       m_Context.state = AppContext::State::Patching;
//       m_LEDMatrixManager.drawAllConnections(m_ModuleBuilder);
//     }
//   }


//   void clearModuleSelections()
//   {
//     m_Context.selectedModule = nullptr;
//     m_Context.selectedDisplayable = nullptr;
//     m_Context.selectedControllable = nullptr;
//     m_Context.selectedAnimatable = nullptr;
//   }

//   void handleDoublePress(sndbx::vector_4U<sndbx::event::TrellisPress>& events)
//   {
//     const auto& firstEvent = events.at(0);
//     const auto& secondEvent = events.at(1);
//     const auto& firstPos = firstEvent.position;
//     const auto& secondPos = secondEvent.position;

//     auto deleteFirstEvent = [](auto& events){ if (!events.is_empty()) events.erase(events.begin()); };

//     if (firstPos == secondPos) 
//     { 
//       deleteFirstEvent(events); 
//       return;
//     }

//     if (sndbx::grid::isBuildableArea(firstPos) && sndbx::grid::isBuildableArea(secondPos)) //Grid -> grid
//     {
//       constexpr static auto patchActionTimeout = 1500ul;
//       if (secondEvent.time - firstEvent.time < patchActionTimeout) 
//       {
//         patchHandler(firstPos, secondPos);
//         events.clear();
//       }
//       else { deleteFirstEvent(events); }
//     }
//     else if (sndbx::grid::isBankArea(firstPos) && sndbx::grid::isBuildableArea(secondPos)) // bank -> grid
//     {
//       m_Context.state = AppContext::State::Creating;
//       createModule(firstPos, secondPos);
//       events.clear();
//     }
//     else { deleteFirstEvent(events); }
//   }

//   void handleGridPress(const sndbx::event::TrellisPress& event, sndbx::vector_4U<sndbx::event::TrellisPress>& events)
//   {
//     if (events.size() == 2) { return handleDoublePress(events); }

//     const auto entry = m_ModuleBuilder.getEntry(event.position);
//     if (!entry) { return; }

//     clearModuleSelections();

//     m_Context.selectedModule = m_ModuleBuilder.getFromEntry<Module>(*entry);
//     m_Context.selectedControllable = m_ModuleBuilder.getFromEntry<Controllable>(*entry); //no check needed, if module isn't controllable then selection set to null

//     if (auto pressable = m_ModuleBuilder.getFromEntry<Pressable>(*entry)) 
//     { 
//       pressable->onRisingEdge(); 
//       m_LEDMatrixManager.markDirty();
//     }

//     if (const auto displayable = m_ModuleBuilder.getFromEntry<Displayable>(*entry))
//     {
//       m_Context.selectedDisplayable = displayable;
//       m_Context.state = AppContext::State::Displaying;
//       m_ScreenManager.markDirty();
//     }

//     if (const auto animatable = m_ModuleBuilder.getFromEntry<Animatable>(*entry))
//     {
//       m_Context.selectedAnimatable = animatable;
//       m_Context.state = AppContext::State::Displaying;
//     }
//   }

//   void handleTrellisRisingEdge(const sndbx::event::TrellisPress& event)
//   {
//     m_Context.trellisEvents.push_back(event);
//     handleGridPress(event, m_Context.trellisEvents);
//   }

//   void handleTrellisFallingEdge(const sndbx::event::TrellisPress& event)
//   {
//     if (auto pressableModule = m_ModuleBuilder.get<Pressable>(event.position))
//     {
//       pressableModule->onFallingEdge();
//       m_LEDMatrixManager.markDirty();
//     }
//   }

//   void handleLongPress(const sndbx::event::TrellisPress& event)
//   {
//     switch (m_Context.mode)
//     {
//       case AppContext::Mode::Edit: 
//         handleDeleteModule(event.position); 
//         clearModuleSelections();
//         break;
//       case AppContext::Mode::View: 
//         break;
//     }
//   }

//   void handleTrellisPress(const sndbx::event::TrellisPress& event)
//   {
//     static Timer holdTimer;
//     constexpr static auto holdThresholdMs = 1000;

//     const auto edge = event.edge;

//     if (edge == sndbx::event::Edge::RISING_EDGE)
//     {
//       holdTimer.start();
//       handleTrellisRisingEdge(event); 
//     }
//     else if (edge == sndbx::event::Edge::FALLING_EDGE) 
//     { 
//       if (holdTimer.hasReached(holdThresholdMs)) { handleLongPress(event); }
//       handleTrellisFallingEdge(event);
//     }
//   }

//   [[nodiscard]] float processorUsage()
//   {
//     float total{};
//     for (const auto& entry : m_ModuleBuilder.registry()) 
//     { 
//       total += entry.module->audio().processorUsage(); 
//     }
//     return total;
//   }

//   void readInputs()
//   {
//     m_Trellis.update();
//     m_Trellis.update();

//     if (m_Trellis.hasEvent()) { handleTrellisPress(*(m_Trellis.popEvent())); }
//   }

//   void updateAppState() 
//   {
//     switch (m_Context.state)
//     {
//       case AppContext::State::Idle:
//       case AppContext::State::Patching: break;
//       case AppContext::State::Creating:
//       case AppContext::State::Deleting:
//       case AppContext::State::Displaying:
//         m_ScreenManager.displaySelectedModule(m_Context, m_ModuleBuilder);
//         break;
//       case AppContext::State::Selecting:
//       default: break;
//     }
//   }

//   void renderDisplay()
//   {
//     m_LEDMatrixManager.renderFrame(m_Context, m_ModuleBuilder);
//     m_Screen.renderFrame();
//   }

//   void loop() 
//   {
//     readInputs();
//     updateAppState();
//     renderDisplay();
//   }

// private:
//   static App* app;

//   AppContext m_Context;

//   TFT m_Screen;
//   ScreenManager m_ScreenManager{m_Screen};
//   Trellis m_Trellis;
//   LEDMatrixDisplay m_LEDMatrix{m_Trellis};
//   LEDMatrixManager m_LEDMatrixManager{m_LEDMatrix};

//   Encoders m_Encoders{onEncoderTurn};

//   ModuleBuilder m_ModuleBuilder;
//   AudioGraph m_AudioGraph;
// };