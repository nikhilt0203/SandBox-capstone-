#ifndef SANDBOX_TIMER_HPP_
#define SANDBOX_TIMER_HPP_

#include <Arduino.h>

class Timer {
public:
  Timer() = default;

  void start() { m_LastTime = millis(); }
  void set(std::uint32_t ms) { m_LastTime = ms; }

  [[nodiscard]] bool hasReached(std::uint32_t ms) const {
    return millis() - m_LastTime > ms;
  }
  [[nodiscard]] std::uint32_t read() const { return millis() - m_LastTime; }

private:
  std::uint32_t m_LastTime{};
};

#endif