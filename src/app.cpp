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
// #include <Arduino.h>
// #include "sd_card.hpp"
// namespace sndbx::engine
// {
//   inline AudioGraph audioGraph;
//   inline ModuleBuilder builder;
// }

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

//   enum class Mode
//   {
//     Edit,
//     View
//   };

//   Mode mode{Mode::Edit};
//   State state{State::Idle};

//   Module* selectedModule{};
//   Displayable* selectedDisplayable{};
//   Controllable* selectedControllable{};
//   Animatable* selectedAnimatable{};

//   sndbx::vector_4U<sndbx::event::TrellisPress> trellisEvents{};
// };

// struct DisplayContext
// {
//   std::array<Displayable*, sndbx::grid::totalCells> moduleDisplays{};
//   Displayable* lastDisplayed{};
//   bool ledsUpdated{false};
//   bool displayUpdated{false};
// };

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

// inline AppContext g_AppContext;
// inline DisplayContext g_DisplayContext;
// inline ModuleBankDisplay g_ModuleBankDisplay;
// inline KeyboardManager g_KeyboardManager;

// namespace sndbx::input
// {
//   void onTurn(std::size_t encoderNum, int delta);
//   inline Encoders encoders(onTurn);
//   inline Trellis trellis;
// }

// namespace sndbx::display
// {
//   inline TFT screen;
//   inline LEDMatrixDisplay ledMatrix(input::trellis);
// }

// namespace sndbx::input
// {
//   void turnBank(int delta)
//   {
//     const auto newStartIndex = 
//         (static_cast<int>(g_ModuleBankDisplay.startIndex) + delta) % 
//         static_cast<int>(g_ModuleBankDisplay.colors.size());

//     g_ModuleBankDisplay.startIndex = newStartIndex;
//     ::g_DisplayContext.ledsUpdated = false;
//   }

//   void onTurn(std::size_t encoderNum, int delta) 
//   {
//     const auto& buttonPresses = ::g_AppContext.trellisEvents;

//     if (!buttonPresses.is_empty() && grid::isBankArea(buttonPresses.at(0).position)) 
//     { 
//       turnBank(delta);
//       return;
//     }
    
//     if (auto selectedModule = ::g_AppContext.selectedControllable) 
//     {
//       selectedModule->changeControl(encoderNum, delta);
//       ::g_DisplayContext.displayUpdated = false;
//       ::g_DisplayContext.ledsUpdated = false;
//     }
//   }
// }

// namespace sndbx::app
// {
//   void init()
//   {
//     Serial.begin(115200);
//     sndbx::sdcard::init();
//     ui::clear(display::ledMatrix);
//     ui::clearAndDraw<SplashScreen>(display::screen);
//     // sndbx::sdcard::writeLine("hello bruh \n poop", "test.txt");
//     // Serial.println(sndbx::sdcard::fileContents("test.txt").at(1).data());
//     //ui::clearAndDraw<ErrorDisplay>(display::screen, "cannot place module here.");
//   }

//   void placeModule(Displayable* module, grid::Position pos)
//   {
//     ::g_DisplayContext.moduleDisplays[pos.index()] = module;
//     display::ledMatrix.drawPixel(pos, module->ledColor());
//     ::g_DisplayContext.ledsUpdated = false;
//   }

//   [[nodiscard]] sndbx::vector_8U<std::uint32_t> connectedModuleColors(const Module::PortArray& ports, ModuleBuilder& builder)
//   {
//     sndbx::vector_8U<std::uint32_t> portColors;

//     for (const auto& port : ports)
//     {
//       if (!port.connectedModule) 
//       {
//         portColors.push_back(0);
//         continue;
//       }

//       const auto connectedID = port.connectedModule->id();

//       if (auto connectedModule = builder.get<Displayable>(connectedID))
//       {
//         portColors.push_back(connectedModule->displayColor());
//       }
//     }

//     return portColors;
//   }
  
//   void displayModule(Displayable* moduleDisplay, Module* module) 
//   {
//     ui::clearAndDraw<ModuleDisplay>(
//       display::screen,
//       moduleDisplay->displayName(),
//       moduleDisplay->displayColor(),
//       moduleDisplay->controlNames(),
//       moduleDisplay->normalizedControlValues(),
//       moduleDisplay->inputNames(),
//       connectedModuleColors(module->inputs(), engine::builder),
//       moduleDisplay->outputNames(),
//       connectedModuleColors(module->outputs(), engine::builder)
//     );
//   }
  
  
//   bool addKeyboardKey(Keyboard& keyboard) 
//   {
//     if (auto keyData = g_KeyboardManager.addKey(keyboard, engine::builder))
//     {
//       placeModule(keyData->key, keyData->position);
//       return true;
//     }
//     return false;
//   }

//   bool deleteModule(sndbx::grid::Position pos);
//   bool subtractKeyboardKey(Keyboard& keyboard) { return g_KeyboardManager.subtractKey(keyboard, deleteModule); }
  
//   void changeKeyboardScale(Keyboard::Scale scale, std::uint32_t keyboardID) { g_KeyboardManager.changeScale(scale, keyboardID); }

//   void handleDeleteModule(grid::Position pos)
//   {
//     if (g_KeyboardManager.isKeyboardAt(pos)) { g_KeyboardManager.deleteKeyboard(pos, deleteModule); }
//     if (!g_KeyboardManager.isKeyAt(pos)) { deleteModule(pos); }
//   }

//   void initKeyboard(Keyboard* keyboard, grid::Position pos)
//   {
//     if (!keyboard) { return; }
    
//     keyboard->setAddKeyCallback(addKeyboardKey);
//     keyboard->setSubtractKeyCallback(subtractKeyboardKey);
//     keyboard->setScaleChangeCallback(changeKeyboardScale);
//     g_KeyboardManager.addKeyboard(keyboard->id(), pos);

//     //create initial keys
//     for (std::size_t i{}; i < 8; ++i) { keyboard->changeControl(0, 1); }
//   }

//   bool deleteModule(grid::Position pos)
//   {
//     auto module = engine::builder.get<Module>(pos);
//     if (!module) { return false; }

//     patch::disconnectAll(engine::audioGraph, module);

//     if (!engine::builder.destroy(pos)) { return false; }
  
//     auto& moduleDisplay = ::g_DisplayContext.moduleDisplays;
//     moduleDisplay.at(pos.index()) = nullptr;

//     ::g_DisplayContext.ledsUpdated = false;
//     return true;
//   }

//   void displayError(Error error)
//   {
//     switch (error)
//     {
//       case Error::BUILDER_INVALID_POS:
//         ui::clearAndDraw<ErrorDisplay>(display::screen, "can't place module here");
//         break;
//       case Error::BUILDER_POOL_EXHAUSTED:
//         ui::clearAndDraw<ErrorDisplay>(display::screen, 
//           "can't create more modules \n         of this type");
//         break;
//       case Error::BUILDER_REGISTRY_FULL: 
//         ui::clearAndDraw<ErrorDisplay>(display::screen, "can't create more modules");
//         break;
//       default: Serial.println("No error");
//     }
//   }

//   void displayError(const sndbx::string32_t& message)
//   {
//     ui::clearAndDraw<ErrorDisplay>(display::screen, message.view());
//   }

//   void displayMaxModuleTypeError(const sndbx::engine::ModuleBankEntry& moduleInfo)
//   {
//     ui::clearAndDraw<ErrorDisplay>(display::screen, "can't create another");
//     ui::draw<Text>(display::screen, 0, 70, moduleInfo.name, moduleInfo.color, 1);
//   }

//   void createModule(grid::Position bankPos, grid::Position gridPos)
//   {
//     const auto bankIndex = (bankPos.index() - grid::bankStart + g_ModuleBankDisplay.startIndex)
//       % g_ModuleBankDisplay.colors.size();

//     //Serial.println(bankIndex);
//     if (bankIndex == engine::bankIndexOf<Keyboard>()) 
//     { 
//       const auto result = engine::builder.make<Keyboard>(gridPos);
//       if (result) 
//       { 
//         initKeyboard(result.value, gridPos); 
//       }
//       else if (result.error == Error::BUILDER_POOL_EXHAUSTED) 
//       { 
//         displayMaxModuleTypeError(engine::bankInfos.at(bankIndex)); 
//       }
//       else 
//       {
//         displayError(result.error); 
//       }
//     }
//     else
//     {
//       const auto error = engine::createModuleFromBankIndex(bankIndex, gridPos, engine::builder);
//       if (error == Error::BUILDER_POOL_EXHAUSTED) 
//       { 
//         displayMaxModuleTypeError(engine::bankInfos.at(bankIndex)); 
//       }
//       else { displayError(error); }
//     }
    
//     const auto entry = engine::builder.getEntry(gridPos);
//     assert(entry);

//     ::g_AppContext.selectedModule       = engine::builder.getFromEntry<Module>(*entry);
//     ::g_AppContext.selectedControllable = engine::builder.getFromEntry<Controllable>(*entry);
//     ::g_AppContext.selectedAnimatable   = engine::builder.getFromEntry<Animatable>(*entry);

//     if (auto moduleDisplay = engine::builder.getFromEntry<Displayable>(*entry))
//     {
//       placeModule(moduleDisplay, gridPos);
//       ::g_AppContext.selectedDisplayable = moduleDisplay;
//       ::g_AppContext.state = AppContext::State::Displaying;
//       ::g_DisplayContext.displayUpdated = false;
//     }
//   }

//   //========================================================================================================================
//   // UI
//   //========================================================================================================================
//   [[nodiscard]] std::optional<sndbx::grid::Position> getPosition(const ModuleBuilder& builder, Module* m)  
//   {
//     for (const auto& entry : builder.registry())
//     {
//       if (entry.module == m) { return entry.position; }
//     }

//     return std::nullopt;
//   }

//   void drawConnectionBetween(LEDMatrixDisplay& ledMatrix, grid::Position srcPos, grid::Position destPos)
//   {
//     auto moduleDisplay = engine::builder.get<Displayable>(srcPos);

//     std::uint32_t wireColor = 
//       moduleDisplay 
//       ? sndbx::color::changeBrightness(moduleDisplay->ledColor(), 0.1) 
//       : 0x404040;

//     auto currentRow = srcPos.row;
//     auto currentCol = srcPos.col;

//     while (currentRow != destPos.row)
//     {
//       if (currentRow > destPos.row) { --currentRow; }
//       else if (currentRow < destPos.row) { ++currentRow; }
//       ledMatrix.drawPixel(currentRow, srcPos.col, wireColor);
//     }

//     while (currentCol != destPos.col)
//     {
//       if (currentCol > destPos.col) { --currentCol; }
//       else if (currentCol < destPos.col) { ++currentCol; }
//       ledMatrix.drawPixel(currentRow, currentCol, wireColor);
//     }
//   }

//   void drawAllConnections(LEDMatrixDisplay& ledMatrix)
//   {
//     ui::clear(ledMatrix);

//     for (std::uint8_t i{}; i < ::g_DisplayContext.moduleDisplays.size(); ++i) 
//     {
//       const auto position = grid::toPosition(i);

//       auto module = engine::builder.get<Module>(position);
//       if (!module) { continue; }

//       for (const auto& port : module->outputs())
//       {
//         const auto connectedModule = port.connectedModule;
//         if (!connectedModule) { continue; }

//         if (auto destPos = getPosition(engine::builder, connectedModule))
//         {
//           drawConnectionBetween(ledMatrix, position, *destPos);
//         }
//       }
//     }

//     ::g_DisplayContext.ledsUpdated = false;
//   }

//   //========================================================================================================================
//   // Patching
//   //========================================================================================================================

//   bool handleDisconnect(Module* src, Module* dest)
//   {
//     return sndbx::patch::disconnectFirstConnection(engine::audioGraph, src, dest);
//   }

//   bool handleConnect(Module* src, Displayable* srcDisplay, Module* dest, Displayable* destDisplay)
//   {
//     const auto output = sndbx::patch::firstAvailablePort(src->outputs());
//     const auto input = sndbx::patch::firstAvailablePort(dest->inputs());
//     if (!output || !input) { return false; }

//     bool connectSuccess = sndbx::patch::connect(engine::audioGraph, src, *output, dest, *input);
//     if (!connectSuccess) { return false; }

//     if (!srcDisplay || !destDisplay) { return connectSuccess; }

//     ui::clearAndDraw<PatchDisplayPage>(
//       display::screen,
//       srcDisplay->displayName(),
//       destDisplay->displayName(),
//       srcDisplay->outputNames().at(*output),
//       destDisplay->inputNames().at(*input),
//       srcDisplay->displayColor(),
//       destDisplay->displayColor()
//     );

//     return true;
//   }

//   void clearModuleSelections(AppContext& appContext);
//   void patchHandler(grid::Position srcPos, grid::Position destPos)
//   {
//     const auto srcEntry = engine::builder.getEntry(srcPos);
//     if (!srcEntry) { return; }

//     const auto destEntry = engine::builder.getEntry(destPos);
//     if (!destEntry) { return; }

//     const auto srcModule   = engine::builder.getFromEntry<Module>(*srcEntry);
//     const auto destModule  = engine::builder.getFromEntry<Module>(*destEntry);
//     const auto srcDisplay  = engine::builder.getFromEntry<Displayable>(*srcEntry);
//     const auto destDisplay = engine::builder.getFromEntry<Displayable>(*destEntry);

//      bool patchChanged = 
//       sndbx::patch::connectionExists(srcModule, destModule) 
//       ? handleDisconnect(srcModule, destModule)
//       : handleConnect(srcModule, srcDisplay, destModule, destDisplay);

//     if (patchChanged) 
//     { 
//       clearModuleSelections(::g_AppContext);
//       Serial.println("setting to patching");
//       ::g_AppContext.state = AppContext::State::Patching;
//       drawAllConnections(display::ledMatrix);
//     }
//   }

//   //========================================================================================================================
//   // Selection dispatch
//   //========================================================================================================================

//   void clearModuleSelections(AppContext& appContext)
//   {
//     appContext.selectedModule = nullptr;
//     appContext.selectedDisplayable = nullptr;
//     appContext.selectedControllable = nullptr;
//     appContext.selectedAnimatable = nullptr;
//   }

//   void handleDoublePress(sndbx::vector_4U<event::TrellisPress>& events)
//   {
//     const auto& firstEvent = events.at(0);
//     const auto& secondEvent = events.at(1);
//     const auto& firstPos = firstEvent.position;
//     const auto& secondPos = secondEvent.position;

//     auto clearFirst = [](auto& events){ if (!events.is_empty()) events.erase(events.begin()); };

//     if (firstPos == secondPos) 
//     { 
//       clearFirst(events); 
//       return;
//     }

//     if (grid::isBuildableArea(firstPos) && grid::isBuildableArea(secondPos))
//     {
//       constexpr static auto maxTimeBetweenSelections = 1500ul;
//       if (secondEvent.time - firstEvent.time > maxTimeBetweenSelections) 
//       {
//         clearFirst(events);
//         return;
//       }
//       patchHandler(firstPos, secondPos);
//       events.clear();
//     }
//     else if (grid::isBankArea(firstPos) && grid::isBuildableArea(secondPos))
//     {
//       ::g_AppContext.state = AppContext::State::Creating;
//       createModule(firstPos, secondPos);
//       events.clear();
//     }
//     else
//     {
//       clearFirst(events);
//     }
//   }

//   void handleGridPress(const event::TrellisPress& event, sndbx::vector_4U<event::TrellisPress>& events)
//   {
//     const auto& builder = engine::builder;
//     const auto& position = event.position;

//     if (events.size() == 2) { return handleDoublePress(events); }

//     const auto entry = builder.getEntry(position);
  
//     if (!entry) { return; }

//     clearModuleSelections(::g_AppContext);

//     ::g_AppContext.selectedModule = builder.getFromEntry<Module>(*entry);
//     ::g_AppContext.selectedControllable = builder.getFromEntry<Controllable>(*entry);

//     if (auto pressable = builder.getFromEntry<Pressable>(*entry)) 
//     { 
//       pressable->onRisingEdge(); 
//       ::g_DisplayContext.ledsUpdated = false;
//     }

//     if (auto displayable = builder.getFromEntry<Displayable>(*entry))
//     {
//       ::g_AppContext.selectedDisplayable = displayable;
//       ::g_AppContext.state = AppContext::State::Displaying;
//       ::g_DisplayContext.displayUpdated = false;
//     }

//     if (auto animatable = builder.getFromEntry<Animatable>(*entry))
//     {
//       ::g_AppContext.selectedAnimatable = animatable;
//       ::g_AppContext.state = AppContext::State::Displaying;
//     }
//   }

//   void handleTrellisRisingEdge(const event::TrellisPress& event)
//   {
//     ::g_AppContext.trellisEvents.push_back(event);
//     handleGridPress(event, ::g_AppContext.trellisEvents);
//   }

//   void handleTrellisFallingEdge(const event::TrellisPress& event)
//   {
//     if (auto pressableModule = engine::builder.get<Pressable>(event.position))
//     {
//       pressableModule->onFallingEdge();
//       ::g_DisplayContext.ledsUpdated = false;
//     }
//   }

//   void handleLongPress(const event::TrellisPress& event)
//   {
//     switch (::g_AppContext.mode)
//     {
//       case AppContext::Mode::Edit: 
//         handleDeleteModule(event.position); 
//         clearModuleSelections(::g_AppContext);
//         break;
//       case AppContext::Mode::View: 
//         break;
//     }
//   }

//   void handleTrellisPress(const event::TrellisPress& event)
//   {
//     static Timer holdTimer;
//     constexpr static auto holdThresholdMs = 1000;

//     const auto edge = event.edge;

//     if (edge == event::Edge::RISING_EDGE)
//     {
//       holdTimer.start();
//       handleTrellisRisingEdge(event); 
//     }
//     else if (edge == event::Edge::FALLING_EDGE) 
//     { 
//       if (holdTimer.hasReached(holdThresholdMs)) { handleLongPress(event); }
//       handleTrellisFallingEdge(event);
//     }
//   }

//   void updateLEDs(LEDMatrixDisplay& ledMatrix)
//   {
//     if (!::g_DisplayContext.ledsUpdated) 
//     { 
//       drawAllConnections(ledMatrix);
//       ui::draw<ModuleBank>(ledMatrix, g_ModuleBankDisplay.colors, g_ModuleBankDisplay.startIndex);
//     }

//     const auto& moduleDisplays = ::g_DisplayContext.moduleDisplays;
//     for (std::size_t i{}; i < moduleDisplays.size(); ++i) 
//     { 
//       const auto module = moduleDisplays[i];
//       if (!module) { continue; }

//       auto moduleColor = module->ledColor();
//       const auto& selected = ::g_AppContext.selectedDisplayable;

//       if (selected == module)
//       {
//         moduleColor = sndbx::color::blend(moduleColor, 0xDDDDFF, 0.1);
//       }

//       ledMatrix.drawPixel(grid::toPosition(static_cast<std::uint8_t>(i)), moduleColor);
//     }

//     ledMatrix.renderFrame();
//     ::g_DisplayContext.ledsUpdated = true;
//   }

//   void displaySelectedModule()
//   {
//     if (auto animatable = ::g_AppContext.selectedAnimatable)
//     {
//       animatable->drawNext(display::screen.currentFrame());
//       display::screen.setFrameAvailable(true);
//       return;
//     }

//     if (!::g_DisplayContext.displayUpdated)
//     {
//       if (auto selected = ::g_AppContext.selectedDisplayable)
//       {
//         displayModule(selected, ::g_AppContext.selectedModule);
//         ::g_DisplayContext.displayUpdated = true;
//       }
//     }
//   }

//   float processorUsage()
//   {
//     float total{};
//     for (const auto& entry : engine::builder.registry()) 
//     { 
//       total += entry.module->audio().processorUsage(); 
//     }
//     return total;
//   }

//   void readInputs()
//   {
//     input::trellis.update();
//     input::encoders.update();

//     if (input::trellis.hasEvent()) 
//     {
//       handleTrellisPress(*input::trellis.popEvent()); 
//     }
//   }
//   auto oldstate = AppContext::State::Idle;
//   void updateState() 
//   {
//     if (oldstate != ::g_AppContext.state) { Serial.println(static_cast<int>(::g_AppContext.state)); }
//     switch (::g_AppContext.state)
//     {
//       case AppContext::State::Idle:
//       case AppContext::State::Patching: break;
//       case AppContext::State::Creating:
//       case AppContext::State::Deleting:
//       case AppContext::State::Displaying:
//         displaySelectedModule();
//         break;
//       case AppContext::State::Selecting:
//       default: break;
//     }
//     oldstate = ::g_AppContext.state;
//   }

//   void renderDisplay()
//   {
//     updateLEDs(display::ledMatrix);
//     display::screen.renderFrame();
//   }

//   void loop() 
//   {
//     readInputs();
//     updateState();
//     renderDisplay();
//   }
// }