#ifndef NST_ILI9341_DISPLAY_HPP_
#define NST_ILI9341_DISPLAY_HPP_

#include "Adafruit_ILI9341.h"
#include "nst/pin.hpp"
#include <array>
#include <cstdint>

namespace nst::teensy {

class TFT {
  public:
	static constexpr std::size_t width = 320;
	static constexpr std::size_t height = 240;

	TFT(Pin cs_pin, Pin dc_pin) : tft_(cs_pin.value, dc_pin.value) {
		tft_.begin();
		tft_.setRotation(3);
		tft_.fillScreen(0);
	}

	void render_frame() {
		auto &prev_buffer = *double_buffer_[1];
		auto &cur_buffer = *double_buffer_[0];

		const auto prev_data = prev_buffer.getBuffer();
		const auto cur_data = cur_buffer.getBuffer();

		constexpr static auto row_size_bytes =
		    sizeof(std::uint16_t) * TFT::width;

		for (std::size_t row{}; row < TFT::height; ++row) {
			const auto row_offset = TFT::width * row;
			const auto cur_row = cur_data + row_offset;
			const auto prev_row = prev_data + row_offset;

			const auto row_diff =
			    std::memcmp(cur_row, prev_row, row_size_bytes);

			if (row_diff) {
				tft_.drawRGBBitmap(0, row, cur_row, TFT::width, 1);
				std::memcpy(prev_row, cur_row, row_size_bytes);
			}
		}

		auto *tmp = double_buffer_[0];
		double_buffer_[0] = double_buffer_[1];
		double_buffer_[1] = tmp;
	}

	void clear() { current_frame().fillScreen(0); }

	[[nodiscard]] GFXcanvas16 &current_frame() { return *double_buffer_[0]; }

	template <typename U> void for_each_buffer(U &&f) {
		for (auto b : double_buffer_) {
			f(b);
		}
	}

	[[nodiscard]] auto &tft() { return tft_; }

  private:
	Adafruit_ILI9341 tft_;

	GFXcanvas16 buffer1_{width, height};
	GFXcanvas16 buffer2_{width, height};
	std::array<GFXcanvas16 *, 2> double_buffer_{&buffer1_, &buffer2_};
};

} // namespace nst::teensy

#endif