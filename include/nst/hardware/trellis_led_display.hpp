#ifndef NST_TRELLIS_LED_DISPLAY_HPP_
#define NST_TRELLIS_LED_DISPLAY_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "led_frame.hpp"
#include "trellis.hpp"
#include "ui/color.hpp"
#include "ui/led_ui_element.hpp"

namespace nst::teensy {

template <class Trellis> class TrellisLEDDisplay {
  static_assert(std::is_same_v<Adafruit_MultiTrellis, Trellis> ||
                std::is_same_v<Adafruit_NeoTrellis, Trellis>,
                "Must use type Adafruit_NeoTrellis or Adafruit_MultiTrellis");

public:
  explicit TrellisLEDDisplay(Trellis &trellis) : trellis_{trellis} {
    buffer1_.clear();
    buffer2_.clear();
  }

  [[nodiscard]] LEDFrame &current_frame() { return *framebuffers_[0]; }
  [[nodiscard]] const LEDFrame &current_frame() const { return *framebuffers_[0]; }

  void clear() { current_frame().clear(); }

  void draw_pixel(std::uint16_t x, std::uint16_t y, std::uint32_t color) {
    current_frame().draw_pixel(x, y, color);
  }

  void draw_pixel(std::size_t index, std::uint32_t color) {
    current_frame()[index] = color;
  }

  void render_frame() {
    if (!frame_available_) {
      return;
    }

    auto &previous_frame = *framebuffers_[1];
    const auto &current_frame = *framebuffers_[0];

    for (std::size_t px{}; px < LEDFrame::size; ++px) {
      const auto current_color = current_frame.at(px);
      if (current_color == previous_frame.at(px)) {
        continue;
      }

      const auto color = nst::color::brightness(current_color, brightness_);
      trellis_.setPixelColor(px, color);

      previous_frame.at(px) = current_color;
    }

    trellis_.show();
    swap_frames();
    frame_available_ = false;
  }

  void push_frame() { frame_available_ = true; }

  void brightness(float brightness) { brightness_ = brightness; }

private:
  void swap_frames() {
    auto *tmp = framebuffers_[0];
    framebuffers_[0] = framebuffers_[1];
    framebuffers_[1] = tmp;
  }

private:
  Trellis &trellis_;
  LEDFrame buffer1_;
  LEDFrame buffer2_;
  std::array<LEDFrame *, 2> framebuffers_{&buffer1_, &buffer2_};
  bool frame_available_{true};
  float brightness_{0.5f};
};

} // namespace nst::teensy

#endif