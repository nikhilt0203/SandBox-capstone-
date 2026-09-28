#include "ILI9341_display.hpp"
#include "Fonts/FreeSansBoldOblique9pt7b.h"
#include <cstring>

void init_buffer(GFXcanvas16 &buffer) {
  buffer.setFont(&FreeSansBoldOblique9pt7b);
}

nst::teensy::TFT::TFT(Pin cs_pin, Pin dc_pin)
    : tft_(cs_pin.value, dc_pin.value) {
  tft_.begin();
  tft_.setRotation(3);
  tft_.fillScreen(0);
  init_buffer(buffer1_);
  init_buffer(buffer2_);
}

void nst::teensy::TFT::render_frame() {
  auto &prev_buffer = *double_buffer_[1];
  auto &cur_buffer = *double_buffer_[0];

  const auto prev_data = prev_buffer.getBuffer();
  const auto cur_data = cur_buffer.getBuffer();

  constexpr static auto row_size_bytes = sizeof(std::uint16_t) * TFT::width;

  for (std::size_t row{}; row < TFT::height; ++row) {
    const auto row_offset = TFT::width * row;
    const auto cur_row = cur_data + row_offset;
    const auto prev_row = prev_data + row_offset;

    const auto row_diff = std::memcmp(cur_row, prev_row, row_size_bytes);

    if (row_diff) {
      tft_.drawRGBBitmap(0, row, cur_row, TFT::width, 1);
      std::memcpy(prev_row, cur_row, row_size_bytes);
    }
  }

  auto *tmp = double_buffer_[0];
  double_buffer_[0] = double_buffer_[1];
  double_buffer_[1] = tmp;
}