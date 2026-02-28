#ifndef SANDBOX_LED_UI_ELEMENTS_HPP_
#define SANDBOX_LED_UI_ELEMENTS_HPP_

#include "ui/led_matrix.hpp"
#include <cstdint>
#include "grid.hpp"

//==========================================================================================
// Base class for all LED UI elements. Derived classes must take in an LEDFrame reference
// as the last argument in their constructor.
//==========================================================================================
class LEDUIElement
{
public:
  LEDUIElement(LEDFrame& frame) : m_Frame(frame) {}

  virtual void draw() const = 0;

protected:
  LEDFrame& m_Frame;
};

//==========================================================================================
// For displaying the module bank
//==========================================================================================
class ModuleBank : public LEDUIElement
{
public:
  ModuleBank(const std::vector<std::uint32_t>& colors, std::size_t startIndex, LEDFrame& frame)
  : LEDUIElement(frame),
    m_Colors(colors),
    m_StartIndex(startIndex)
  {}

  void draw() const override 
  {
    constexpr static auto bankRow = sndbx::grid::bankStart / sndbx::grid::rows;

    const auto numColors = m_Colors.size();
    const auto max = std::min(numColors, sndbx::grid::cols);
    
    for (std::size_t col{}; col < max; col++)
    {
      const auto wrappedIndex = (m_StartIndex + col) % numColors;
      m_Frame.drawPixel(bankRow, col, m_Colors.at(wrappedIndex));
    }
  }

private:
  const std::vector<std::uint32_t>& m_Colors;
  const std::size_t m_StartIndex;
};

#endif