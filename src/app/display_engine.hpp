#ifndef SANDBOX_DISPLAY_ENGINE_HPP_
#define SANDBOX_DISPLAY_ENGINE_HPP_

#include "io/pinouts.hpp"
#include "modules/displayable.hpp"
#include <nst/hardware/ILI9341_display.hpp>
#include <nst/hardware/trellis_led_display.hpp>

namespace sndbx {

using Screen = nst::teensy::TFT;
using LEDGrid = nst::teensy::TrellisLEDDisplay<Adafruit_MultiTrellis>;

class DisplayEngine {
public:
  explicit DisplayEngine(Adafruit_MultiTrellis &t) : led_grid_{t} {}
  
  void render_frame() { screen_.render_frame(); }

  void display_module(const Displayable &d);

private:
  Screen screen_{pinouts::tft_cs, pinouts::tft_dc};
  LEDGrid led_grid_;
};

} // namespace sndbx
#endif