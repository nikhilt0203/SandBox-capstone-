#ifndef SANDBOX_BUTTONS_HPP_
#define SANDBOX_BUTTONS_HPP_

#include "StevesAwesomeButton.h"
#include "config.hpp"
#include "event.hpp"
#include "nst/inplace_vector.hpp"
#include <array>

class Buttons {
  static constexpr std::size_t numButtons = 4;

public:
  Buttons() { s_Instance = this; }

  void update() {
    for (auto &button : m_Buttons) {
      button.process();
    }
  }

  void enableRisingEdge(std::size_t buttonNum) {
    m_Buttons.at(buttonNum).pressHandler(queueRisingEdge);
  }
  void enableFallingEdge(std::size_t buttonNum) {
    m_Buttons.at(buttonNum).releaseHandler(queueFallingEdge);
  }

  void enableRisingEdge() {
    for (auto &button : m_Buttons) {
      button.pressHandler(queueRisingEdge);
    }
  }
  void enableFallingEdge() {
    for (auto &button : m_Buttons) {
      button.releaseHandler(queueFallingEdge);
    }
  }

  [[nodiscard]] bool hasEvent() const { return !m_EventQueue.is_empty(); }

  /**
   * @brief Pop the last turn event.
   *
   * Must check hasEvent() before performing this operation.
   *
   * @return sndbx::event::EncoderTurn
   */
  [[nodiscard]] sndbx::event::ButtonPress popEvent() {
    auto event = m_EventQueue.back();
    m_EventQueue.pop_back();
    return event;
  }

  /**
   * @brief View the last turn event.
   *
   * Must check hasEvent() before performing this operation.
   *
   * @return sndbx::event::EncoderTurn
   */
  [[nodiscard]] const sndbx::event::ButtonPress &readEvent() {
    return m_EventQueue.back();
  }

  [[nodiscard]] constexpr std::size_t size() const { return numButtons; }

private:
  static void queueFallingEdge(int buttonNum) {
    s_Instance->m_EventQueue.emplace_back(buttonNum,
                                          sndbx::event::Edge::FALLING_EDGE);
  }

  static void queueRisingEdge(int buttonNum) {
    s_Instance->m_EventQueue.emplace_back(buttonNum,
                                          sndbx::event::Edge::RISING_EDGE);
  }

private:
  static inline Buttons *s_Instance{};

  // template <std::size_t... I>
  // static auto make_button_array(std::index_sequence<I...>) {
  //   return std::array<StevesAwesomeButton, sizeof...(I)>{StevesAwesomeButton{
  //       static_cast<nst::Pin::underlying_t>(PinArray[I]), I,
  //       INPUT_PULLUP}...};
  // };

  std::array<StevesAwesomeButton, numButtons> m_Buttons{
      StevesAwesomeButton{BUTTON_PIN_1, 0, INPUT_PULLUP},
      StevesAwesomeButton{BUTTON_PIN_2, 1, INPUT_PULLUP},
      StevesAwesomeButton{BUTTON_PIN_3, 2, INPUT_PULLUP},
      StevesAwesomeButton{BUTTON_PIN_4, 3, INPUT_PULLUP}};

  nst::vector_16U<sndbx::event::ButtonPress> m_EventQueue;
};

#endif