#ifndef NST_TRELLIS_LED_DISPLAY_HPP_
#define NST_TRELLIS_LED_DISPLAY_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "led_frame.hpp"
#include "nst/color.hpp"
#include "trellis.hpp"

namespace nst::teensy {

template <class Trellis, std::size_t Rows, std::size_t Cols>
class TrellisLEDDisplay {
	static_assert(std::is_same_v<Adafruit_MultiTrellis, Trellis> ||
	                  std::is_same_v<Adafruit_NeoTrellis, Trellis>,
	              "Must use type Adafruit_NeoTrellis or Adafruit_MultiTrellis");
	using Frame = LEDFrame<Rows, Cols>;

  public:
	explicit TrellisLEDDisplay(Trellis &trellis) : trellis_{trellis} {
		buffer1_.clear();
		buffer2_.clear();
	}

	[[nodiscard]] auto &current_frame() { return *framebuffers_[0]; }
	[[nodiscard]] const auto &current_frame() const {
		return *framebuffers_[0];
	}

	void clear() { current_frame().clear(); }

	void draw_pixel(std::uint16_t x, std::uint16_t y, ColorRGB color) {
		current_frame().draw_pixel(x, y, color);
	}

	void draw_pixel(std::size_t index, ColorRGB color) {
		current_frame()[index] = color;
	}

	void render_frame() {
		auto &previous_frame = *framebuffers_[1];
		const auto &current_frame = *framebuffers_[0];

		for (std::size_t px{}; px < Frame::width * Frame::height; ++px) {
			const auto current_color = current_frame.at(px);
			if (current_color == previous_frame.at(px)) {
				continue;
			}

			trellis_.setPixelColor(px, current_color.hex());

			previous_frame.at(px) = current_color;
		}

		trellis_.show();
		swap_frames();
	}

  private:
	void swap_frames() {
		auto *tmp = framebuffers_[0];
		framebuffers_[0] = framebuffers_[1];
		framebuffers_[1] = tmp;
	}

  private:
	Trellis &trellis_;
	Frame buffer1_;
	Frame buffer2_;
	std::array<Frame *, 2> framebuffers_{&buffer1_, &buffer2_};
};

} // namespace nst::teensy

#endif