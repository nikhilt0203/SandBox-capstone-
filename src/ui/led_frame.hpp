#ifndef SANDBOX_LED_FRAME_HPP_
#define SANDBOX_LED_FRAME_HPP_

#include "config.hpp"
#include <array>
#include <cstdint>

class LEDFrame {
  using LEDFrameBuffer =
      std::array<std::uint32_t,
                 sndbx::config::grid_rows * sndbx::config::grid_cols>;

public:
  static constexpr std::size_t size = std::tuple_size<LEDFrameBuffer>::value;

public:
  LEDFrame() = default;

  void draw_pixel(int row, int col, std::uint32_t color) {
    data_.at(row * sndbx::config::grid_cols + col) = color;
  }

  void clear() { data_.fill(0x000000); }

  [[nodiscard]] std::uint32_t &at(std::size_t index) { return data_[index]; }
  [[nodiscard]] const std::uint32_t &at(std::size_t index) const {
    return data_[index];
  }

  [[nodiscard]] std::uint32_t *data() { return data_.data(); }
  [[nodiscard]] LEDFrameBuffer &buffer() { return data_; }

private:
  LEDFrameBuffer data_{};
};

#endif