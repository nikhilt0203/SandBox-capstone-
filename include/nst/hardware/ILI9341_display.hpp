#ifndef NST_ILI9341_DISPLAY_HPP_
#define NST_ILI9341_DISPLAY_HPP_

#include "Adafruit_ILI9341.h"
#include "nst/pin.hpp"
#include <array>
#include <cstdint>
#include <nst/hardware/ui/color.hpp>
#include <string_view>

namespace nst::teensy {

class TFT {
public:
  static constexpr std::size_t width = 320;
  static constexpr std::size_t height = 240;

public:
  TFT(Pin cs_pin, Pin dc_pin);

  void render_frame();

  void clear() { current_frame().fillScreen(0); }

  [[nodiscard]] GFXcanvas16 &current_frame() { return *double_buffer_[0]; }

  [[nodiscard]] auto &tft() { return tft_; }

private:
  Adafruit_ILI9341 tft_;

  GFXcanvas16 buffer1_{width, height};
  GFXcanvas16 buffer2_{width, height};
  std::array<GFXcanvas16 *, 2> double_buffer_{&buffer1_, &buffer2_};
};

} // namespace nst::teensy

#endif