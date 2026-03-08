#ifndef SANDBOX_TIMER_HPP_
#define SANDBOX_TIMER_HPP_

#include <Arduino.h>

class Timer
{
public:
  Timer() = default;

  void start() { m_LastTime = millis(); }
  
  [[nodiscard]] bool hasReached(unsigned long ms) const { return millis() - m_LastTime > ms; }
  [[nodiscard]] unsigned long read() const { return millis() - m_LastTime; }

private:
  unsigned long m_LastTime{};
};

#endif