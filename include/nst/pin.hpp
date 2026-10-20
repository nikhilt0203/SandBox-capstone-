#ifndef NST_PIN_HPP_
#define NST_PIN_HPP_

#include <Arduino.h>
#include <cstdint>

namespace nst {

struct Pin {
	std::uint8_t num;

	constexpr Pin(std::uint8_t num) : num{num} {}
	std::uint8_t read() const { return digitalRead(num); }
	void write(std::uint8_t val) { digitalWrite(num, val); }
	void pin_mode(std::uint8_t mode) { pinMode(num, mode); }
};

} // namespace nst

#endif