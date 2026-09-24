#ifndef NST_LED_FRAME_HPP_
#define NST_LED_FRAME_HPP_

#include <array>
#include <cstdint>

namespace nst::teensy {

class LEDFrame {
public:
  static constexpr std::size_t width = 8;
  static constexpr std::size_t height = 8;

  using LEDFrameBuffer = std::array<std::uint32_t, width * height>;

public:
  void draw_pixel(std::size_t index, std::uint32_t color) {
    framebuffer_[index] = color;
  }

  void draw_pixel(std::size_t x, std::size_t y, std::uint32_t color) {
    framebuffer_[y * width + x] = color;
  }

  void fill(std::uint32_t color) { framebuffer_.fill(color); }

  void clear() { framebuffer_.fill(0); }

  [[nodiscard]] std::uint32_t &operator[](std::size_t index) {
    return framebuffer_[index];
  }

  [[nodiscard]] const std::uint32_t &operator[](std::size_t index) const {
    return framebuffer_[index];
  }

  [[nodiscard]] std::uint32_t &at(std::size_t index) {
    return framebuffer_.at(index);
  }

  [[nodiscard]] const std::uint32_t &at(std::size_t index) const {
    return framebuffer_.at(index);
  }

  [[nodiscard]] std::uint32_t *data() { return framebuffer_.data(); }

  [[nodiscard]] const std::uint32_t *data() const {
    return framebuffer_.data();
  }

  [[nodiscard]] LEDFrameBuffer &buffer() { return framebuffer_; }

  [[nodiscard]] const LEDFrameBuffer &buffer() const { return framebuffer_; }

private:
  LEDFrameBuffer framebuffer_{};
};

} // namespace nst::teensy

#endif