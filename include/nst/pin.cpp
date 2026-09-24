#include <nst/pin.hpp>
#include <Arduino.h>

std::uint8_t nst::Pin::read() const { return digitalRead(value); }  

void nst::Pin::write(std::uint8_t val) { digitalWrite(value, val); }

void nst::Pin::pin_mode(std::uint8_t mode) { pinMode(value, mode); }
