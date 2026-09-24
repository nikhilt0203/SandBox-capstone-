#ifndef NST_LED_UI_ELEMENT_HPP_
#define NST_LED_UI_ELEMENT_HPP_

#include "nst/hardware/led_frame.hpp"

namespace nst::teensy {

class LEDUIElement {
public:
  LEDUIElement(LEDFrame &frame) : m_frame(frame) {}
  virtual void draw() const = 0;

protected:
  LEDFrame &m_frame;
};

} // namespace nst::teensy

#endif