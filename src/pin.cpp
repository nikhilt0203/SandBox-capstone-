#include <Arduino.h>
#include <nst/pin.hpp>

namespace nst {
std::uint8_t pin::read() const { return digitalRead(num); }
void pin::write(std::uint8_t val) { digitalWrite(num, val); }
void pin::pin_mode(std::uint8_t mode) { pinMode(num, mode); }
} // namespace nst