#ifndef SANDBOX_LED_UI_ELEMENTS_HPP_
#define SANDBOX_LED_UI_ELEMENTS_HPP_

#include "config/config.hpp"
#include "ui/led_matrix.hpp"
#include <cstdint>

//==========================================================================================
// Base class for all LED UI elements. Derived classes must take in an LEDFrame
// reference as the last argument in their constructor.
//==========================================================================================
class LEDUIElement {
  public:
	LEDUIElement(LEDFrame &frame) : frame_(frame) {}

	virtual void draw() const = 0;

  protected:
	LEDFrame &frame_;
};

//==========================================================================================
// For displaying the module bank
//==========================================================================================
template <std::size_t N> class ModuleBank : public LEDUIElement {
  public:
	ModuleBank(const nst::inplace_vector<std::uint32_t, N> &colors,
	           std::size_t startIndex, LEDFrame &frame)
	    : LEDUIElement(frame), m_Colors(colors), m_StartIndex(startIndex) {}

	void draw() const override {
		constexpr static auto bankRow = 54 / sndbx::config::grid_rows;

		const auto numColors = m_Colors.size();
		const auto max =
		    std::min<std::uint8_t>(numColors, sndbx::config::grid_cols);

		for (std::size_t col{}; col < max; ++col) {
			const auto wrappedIndex = (m_StartIndex + col) % numColors;
			frame_.draw_pixel(bankRow, col, m_Colors.at(wrappedIndex));
		}
	}

  private:
	const nst::inplace_vector<std::uint32_t, N> &m_Colors;
	const std::size_t m_StartIndex;
};

template <std::size_t N>
ModuleBank(const nst::inplace_vector<std::uint32_t, N> &, std::size_t,
           LEDFrame &) -> ModuleBank<N>;

#endif