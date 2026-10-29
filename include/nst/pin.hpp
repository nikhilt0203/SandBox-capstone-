#ifndef NST_PIN_HPP_
#define NST_PIN_HPP_

#include <Arduino.h>
#include <cstdint>

namespace nst {

struct pin {
	std::uint8_t num;

	constexpr pin(std::uint8_t num) : num{num} {}
	std::uint8_t read() const;
	void write(std::uint8_t val);
	void pin_mode(std::uint8_t mode);
};

} // namespace nst

#endif