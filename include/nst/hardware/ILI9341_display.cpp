#include "ILI9341_display.hpp"
#include "Fonts/FreeSansBoldOblique9pt7b.h"
#include <cstring>

void init_buffer(GFXcanvas16 &buffer) {
  buffer.setFont(&FreeSansBoldOblique9pt7b);
}

nst::teensy::TFT::TFT(Pin cs_pin, Pin dc_pin)
    : m_tft(nst::to_underlying_t(cs_pin), nst::to_underlying_t(dc_pin)) {
  m_tft.begin();
  m_tft.setRotation(3);
  m_tft.fillScreen(0);
  init_buffer(m_buffer1);
  init_buffer(m_buffer2);
}

void nst::teensy::TFT::render_frame() {
  if (!m_frame_available) {
    return;
  }

  auto &prev_buffer = *m_double_frame_buffer[1];
  auto &cur_buffer = *m_double_frame_buffer[0];

  auto *const prev_data = prev_buffer.getBuffer();
  auto *const curr_data = cur_buffer.getBuffer();

  constexpr static auto row_size_bytes = sizeof(std::uint16_t) * TFT::width;

  for (std::size_t row{}; row < TFT::height; ++row) {
    const std::size_t row_offset = TFT::width * row;
    const auto *const cur_row = curr_data + row_offset;
    auto *const prev_row = prev_data + row_offset;

    const auto row_diff = std::memcmp(cur_row, prev_row, row_size_bytes);

    if (row_diff) {
      m_tft.drawRGBBitmap(0, row, cur_row, TFT::width, 1);
      std::memcpy(prev_row, cur_row, row_size_bytes);
    }
  }

  auto *tmp = m_double_frame_buffer[0];
  m_double_frame_buffer[0] = m_double_frame_buffer[1];
  m_double_frame_buffer[1] = tmp;

  m_frame_available = false;
}