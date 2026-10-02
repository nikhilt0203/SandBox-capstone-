#ifndef SANDBOX_COLOR_TEXT_HPP_
#define SANDBOX_COLOR_TEXT_HPP_

#include <cstdint>
#include <nst/color.hpp>
#include <string_view>

// Data wrappers for text with color and/or size
namespace sndbx {

struct ColoredText {
	std::string_view text{};
	nst::teensy::ColorRGB color{};
};

struct SizedColoredText {
	std::string_view text{};
	nst::teensy::ColorRGB color{};
	std::uint8_t size{};
};

} // namespace sndbx
#endif