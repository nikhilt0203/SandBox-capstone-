// #include "input_handler.hpp"
// #include "app_refactor.hpp"

// void EditMode::onEncoderTurn(App &app, const sndbx::event::EncoderTurn &event) {
//   const auto controlNum = event.encoderNum;
//   app.changeSelectedModuleControl(controlNum, event.delta);
// }

// bool isPatchAction(const sndbx::grid::Position &pos1,
//                    const sndbx::grid::Position &pos2) {
//   return sndbx::grid::isBuildableArea(pos1) &&
//          sndbx::grid::isBuildableArea(pos2);
// }

// bool isCreateAction(const sndbx::grid::Position &pos1,
//                     const sndbx::grid::Position &pos2) {
//   return sndbx::grid::isBankArea(pos1) && sndbx::grid::isBuildableArea(pos2);
// }

// void EditMode::trellisRisingEdge(App &app,
//                                  const Trellis::KeyEvent &event) {
//   const auto &currentPos = event.position;

//   app.selectModuleAt(currentPos);
//   m_GridTimes.at(currentPos.index()) = millis();

//   if (!m_FirstPress) {
//     m_FirstPress = event;
//     m_PatchTimer.start();
//     return;
//   }
//   const auto &firstPos = m_FirstPress->position;

//   if (!m_PatchTimer.hasReached(1500) && isPatchAction(firstPos, currentPos)) {
//     app.handlePatchAction(firstPos, currentPos);
//   } else if (isCreateAction(firstPos, currentPos)) {
//     bool success = app.createModule(firstPos, currentPos);
//     if (success) {
//       app.selectModuleAt(currentPos);
//     }
//   }
// }

// void EditMode::trellisFallingEdge(App &app,
//                                   const Trellis::KeyEvent &event) {
//   const auto &position = event.position;
//   const auto lastRisingEdgeTime = m_GridTimes.at(position.index());
//   if (millis() - lastRisingEdgeTime > 1500) {
//     app.deleteModule(position);
//   }
// }

// void EditMode::onTrellisPress(App &app,
//                               const Trellis::KeyEvent &event) {
//   const auto edge = event.edge;

//   app.triggerPressableModule(event.position, edge);

//   if (edge == sndbx::event::Edge::RISING_EDGE) {
//     trellisRisingEdge(app, event);
//   } else if (edge == sndbx::event::Edge::FALLING_EDGE) {
//     trellisFallingEdge(app, event);
//   }
// }

// void ViewMode::onEncoderTurn(App &app, const sndbx::event::EncoderTurn &event) {
//   const auto controlNum = event.encoderNum;
//   app.changeSelectedModuleControl(controlNum, event.delta);
// }

// void ViewMode::onTrellisPress(App &app,
//                               const Trellis::KeyEvent &event) {
//   app.selectModuleAt(event.position);
// }
