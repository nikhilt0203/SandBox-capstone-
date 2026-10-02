#ifndef NST_LED_FRAME_HPP_
#define NST_LED_FRAME_HPP_

#include "nst/color.hpp"
#include <array>
#include <cstdint>

namespace nst::teensy {

template <std::size_t X, std::size_t Y> class LEDFrame {
  public:
	static constexpr std::size_t width = X;
	static constexpr std::size_t height = Y;

	using Buffer = std::array<ColorRGB, width * height>;

	void draw_pixel(std::size_t index, ColorRGB color) {
		buffer_[index] = color;
	}

	void draw_pixel(std::size_t x, std::size_t y, ColorRGB color) {
		buffer_[y * width + x] = color;
	}

	void fill(ColorRGB color) { buffer_.fill(color); }

	void clear() { buffer_.fill(0); }

	[[nodiscard]] auto &operator[](std::size_t index) { return buffer_[index]; }

	[[nodiscard]] const auto &operator[](std::size_t index) const {
		return buffer_[index];
	}

	[[nodiscard]] auto &at(std::size_t index) { return buffer_.at(index); }

	[[nodiscard]] const auto &at(std::size_t index) const {
		return buffer_.at(index);
	}

	[[nodiscard]] auto data() { return buffer_.data(); }
	[[nodiscard]] const auto data() const { return buffer_.data(); }

	[[nodiscard]] Buffer &buffer() { return buffer_; }
	[[nodiscard]] const auto &buffer() const { return buffer_; }

  private:
	Buffer buffer_{};
};

} // namespace nst::teensy

#endif