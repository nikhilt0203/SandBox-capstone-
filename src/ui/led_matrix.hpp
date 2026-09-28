#ifndef SANDBOX_LED_MATRIX_HPP_
#define SANDBOX_LED_MATRIX_HPP_

#include <cstdint>

#include "color.hpp"
#include "led_frame.hpp"

class LEDMatrixDisplay {
public:
  explicit LEDMatrixDisplay(Adafruit_MultiTrellis &trellis)
      : trellis_(trellis) {
    buffer1_.clear();
    buffer2_.clear();
  }

  [[nodiscard]] LEDFrame &current_frame() { return *double_buffer_[0]; }

  void clear() { current_frame().clear(); }

  void set_frame_available(bool available) { render_next_ = available; }

  void draw_pixel(int row, int col, std::uint32_t color) {
    current_frame().draw_pixel(row, col, color);
    render_next_ = true;
  }

  void render_frame() {
    if (!render_next_) {
      return;
    }

    auto &prev = *double_buffer_[1];
    const auto &cur = *double_buffer_[0];

    for (std::size_t px{}; px < LEDFrame::size; ++px) {
      const auto cur_px = cur.at(px);
      if (cur_px == prev.at(px)) {
        continue;
      }

      const auto color = sndbx::color::brightness(cur_px, brightness_);
      trellis_.setPixelColor(px, color);

      prev.at(px) = cur_px;
    }

    trellis_.show();
    swap_frames();
    render_next_ = false;
  }

  void brightness(float brightness) { brightness_ = brightness; }

private:
  void swap_frames() {
    auto *tmp = double_buffer_[0];
    double_buffer_[0] = double_buffer_[1];
    double_buffer_[1] = tmp;
  }

private:
  Adafruit_MultiTrellis &trellis_;
  LEDFrame buffer1_;
  LEDFrame buffer2_;
  std::array<LEDFrame *, 2> double_buffer_{&buffer1_, &buffer2_};
  bool render_next_{};
  float brightness_{0.5f};
};

#endif