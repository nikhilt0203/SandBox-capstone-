#ifndef NST_PIN_HPP_
#define NST_PIN_HPP_

#include <Arduino.h>
#include <cstdint>
#include <nst/strong_alias.hpp>

namespace nst {

struct Pin : public nst::strong_alias<std::uint8_t, Pin> {
	using strong_alias::strong_alias;
	std::uint8_t read() const { return digitalRead(value); }
	void write(std::uint8_t val) { digitalWrite(value, val); }
	void pin_mode(std::uint8_t mode) { pinMode(value, mode); }
};

} // namespace nst

#endif