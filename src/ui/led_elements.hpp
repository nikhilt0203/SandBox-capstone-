#ifndef SANDBOX_LED_UI_ELEMENTS_HPP_
#define SANDBOX_LED_UI_ELEMENTS_HPP_

#include <cstdint>
#include <nst/hardware/trellis_led_display.hpp>
#include <nst/inplace_vector.hpp>

#include "config/config.hpp"
#include "ui/ui_element.hpp"

namespace sndbx {

using LEDFrame = nst::teensy::LEDFrame<limits::grid_rows, limits::grid_cols>;
using LEDElement = UIElement<LEDFrame, LEDFrame::width, LEDFrame::height>;

//==========================================================================================
// For displaying the module bank
//==========================================================================================
template <std::size_t N>
class ModuleBank : public LEDUIElement {
   public:
	ModuleBank(const nst::inplace_vector<nst::teensy::ColorRGB, N>& colors,
	           std::size_t start_idx, LEDFrame& frame)
	    : LEDUIElement{frame}, colors_{colors}, start_idx_{start_idx} {}

	void draw(LEDFrame& frame = frame_) const {
		constexpr static auto bank_row = 54 / limits::grid_rows;

		const auto max =
		    std::min<std::uint8_t>(colors_.size(), limits::grid_cols);

		for (std::size_t col{}; col < max; ++col) {
			const auto idx = (start_idx_ + col) % colors_.size();
			frame.draw_pixel(bank_row, col, colors_.at(idx));
		}
	}

   private:
	const nst::inplace_vector<nst::teensy::ColorRGB, N>& colors_;
	std::size_t start_idx_;
};

}  // namespace sndbx

#endif