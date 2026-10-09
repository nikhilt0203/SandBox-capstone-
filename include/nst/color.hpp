#ifndef NST_TEENSY_COLOR_HPP_
#define NST_TEENSY_COLOR_HPP_

#include <cstdint>

// Color structs for 565 and RGB format. Defines conversions between the two.
// All functionality can (and should, if possible) be used at compile time
namespace nst::teensy {

struct ColorRGB;
struct Color565 {
	std::uint8_t r : 5;
	std::uint8_t g : 6;
	std::uint8_t b : 5;

	constexpr Color565() : r{0}, g{0}, b{0} {}

	constexpr Color565(std::uint32_t hex) noexcept
	    : r{static_cast<std::uint8_t>((hex >> 19) & 0x1F)},
	      g{static_cast<std::uint8_t>((hex >> 10) & 0x3F)},
	      b{static_cast<std::uint8_t>((hex >> 3) & 0x1F)} {}

	constexpr Color565(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
	    : r{static_cast<std::uint8_t>(r & 0x1F)},
	      g{static_cast<std::uint8_t>(g & 0x3F)},
	      b{static_cast<std::uint8_t>(b & 0x1F)} {}

	[[nodiscard]] constexpr std::uint16_t hex() const noexcept {
		return (r << 11) | (g << 5) | b;
	}

	constexpr Color565 &operator*=(float f) noexcept {
		r = static_cast<std::uint8_t>(r * f) & 0x1F;
		g = static_cast<std::uint8_t>(g * f) & 0x3F;
		b = static_cast<std::uint8_t>(b * f) & 0x1F;
		return *this;
	}

	constexpr Color565 operator*(float f) const noexcept {
		std::uint8_t r_ = static_cast<std::uint8_t>((r * f)) & 0x1F;
		std::uint8_t g_ = static_cast<std::uint8_t>((g * f)) & 0x3F;
		std::uint8_t b_ = static_cast<std::uint8_t>((b * f)) & 0x1F;
		return {r_, g_, b_};
	}

	explicit constexpr operator ColorRGB() const noexcept;

	explicit operator std::uint16_t() const noexcept { return hex(); }

	constexpr bool operator==(const Color565 &rhs) const noexcept {
		return hex() == rhs.hex();
	}

	constexpr bool operator!=(const Color565 &rhs) const noexcept {
		return hex() != rhs.hex();
	}

	constexpr bool operator==(std::uint16_t rhs) const noexcept {
		return hex() == rhs;
	}

	constexpr bool operator!=(std::uint16_t rhs) const noexcept {
		return hex() != rhs;
	}
};

struct ColorRGB {
	std::uint8_t r{};
	std::uint8_t g{};
	std::uint8_t b{};

	constexpr ColorRGB() = default;

	constexpr ColorRGB(std::uint32_t hex) noexcept
	    : r{static_cast<std::uint8_t>((hex >> 16) & 0xFF)},
	      g{static_cast<std::uint8_t>((hex >> 8) & 0xFF)},
	      b{static_cast<std::uint8_t>(hex & 0xFF)} {}

	constexpr ColorRGB(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept
	    : r{r}, g{g}, b{b} {}

	[[nodiscard]] constexpr std::uint32_t hex() const noexcept {
		return (static_cast<std::uint32_t>(r) << 16) |
		       (static_cast<std::uint32_t>(g) << 8) |
		       static_cast<std::uint32_t>(b);
	}

	constexpr ColorRGB &operator*=(float f) noexcept {
		r = static_cast<std::uint8_t>(r * f);
		g = static_cast<std::uint8_t>(g * f);
		b = static_cast<std::uint8_t>(b * f);
		return *this;
	}

	constexpr Color565 operator*(float f) const noexcept {
		return {static_cast<std::uint8_t>(r * f),
		        static_cast<std::uint8_t>(g * f),
		        static_cast<std::uint8_t>(b * f)};
	}

	constexpr operator Color565() const noexcept {
		return Color565{static_cast<std::uint8_t>(r >> 3),
		                static_cast<std::uint8_t>(g >> 2),
		                static_cast<std::uint8_t>(b >> 3)};
	}

	explicit operator std::uint32_t() const noexcept { return hex(); }

	constexpr bool operator==(const ColorRGB &rhs) const noexcept {
		return hex() == rhs.hex();
	}

	constexpr bool operator!=(const ColorRGB &rhs) const noexcept {
		return hex() != rhs.hex();
	}

	constexpr bool operator==(std::uint32_t rhs) const noexcept {
		return hex() == rhs;
	}

	constexpr bool operator!=(std::uint32_t rhs) const noexcept {
		return hex() != rhs;
	}
};

[[nodiscard]] constexpr ColorRGB blend(ColorRGB c1, ColorRGB c2,
                                       float ratio) noexcept {

	return ColorRGB{static_cast<std::uint8_t>(c1.r * (1 - ratio) + c2.r),
	                static_cast<std::uint8_t>(c1.g * (1 - ratio) + c2.g),
	                static_cast<std::uint8_t>(c1.r * (1 - ratio) + c2.b)};
}

[[nodiscard]] constexpr Color565 blend(Color565 c1, Color565 c2,
                                       float ratio) noexcept {

	return Color565{static_cast<std::uint8_t>(c1.r * (1 - ratio) + c2.r),
	                static_cast<std::uint8_t>(c1.g * (1 - ratio) + c2.g),
	                static_cast<std::uint8_t>(c1.r * (1 - ratio) + c2.b)};
}

constexpr Color565::operator ColorRGB() const noexcept {
	return ColorRGB{static_cast<std::uint8_t>((r << 3) | (r >> 2)),
	                static_cast<std::uint8_t>((g << 2) | (g >> 4)),
	                static_cast<std::uint8_t>((b << 3) | (b >> 2))};
}

[[nodiscard]] constexpr auto to_565_hex(ColorRGB color) {
	return static_cast<Color565>(color).hex();
}

[[nodiscard]] constexpr auto to_rgb_hex(Color565 color) {
	return static_cast<ColorRGB>(color).hex();
}

[[nodiscard]] constexpr auto to_565(ColorRGB color) {
	return static_cast<Color565>(color);
}

[[nodiscard]] constexpr auto to_rgb(Color565 color) {
	return static_cast<ColorRGB>(color);
}

} // namespace nst::teensy

#endif