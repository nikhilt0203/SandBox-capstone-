#ifndef SANDBOX_TIMER_HPP_
#define SANDBOX_TIMER_HPP_

#include <Arduino.h>
#include <cstdint>

class Timer {
public:
  Timer() = default;

  void start() { last_time_ = millis(); }
  void set(std::uint32_t ms) { last_time_ = ms; }

  [[nodiscard]] bool has_reached(std::uint32_t ms) const {
    return millis() - last_time_ > ms;
  }
  [[nodiscard]] std::uint32_t read() const { return millis() - last_time_; }

private:
  std::uint32_t last_time_{};
};

#endif