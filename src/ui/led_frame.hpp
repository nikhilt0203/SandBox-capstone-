#ifndef SANDBOX_LED_FRAME_HPP_
#define SANDBOX_LED_FRAME_HPP_

#include "grid.hpp"
#include <cstdint>
#include <array>

class LEDFrame
{
  using LEDFrameBuffer = std::array<std::uint32_t, sndbx::grid::totalCells>;
public:
  static constexpr std::size_t size = std::tuple_size<LEDFrameBuffer>::value;

public:
  LEDFrame() = default;

  void drawPixel(sndbx::grid::Position pos, std::uint32_t color) { m_FrameBuffer[pos.index()] = color; }
  void drawPixel(int row, int col, std::uint32_t color) { m_FrameBuffer.at(row * sndbx::grid::cols + col) = color; }

  void clear() { m_FrameBuffer.fill(0x000000); }

  [[nodiscard]] std::uint32_t& at(std::size_t index) { return m_FrameBuffer[index]; }
  [[nodiscard]] const std::uint32_t& at(std::size_t index) const { return m_FrameBuffer[index]; }

  [[nodiscard]] std::uint32_t* data() { return m_FrameBuffer.data(); }
  [[nodiscard]] LEDFrameBuffer& buffer() { return m_FrameBuffer; }

private:
  LEDFrameBuffer m_FrameBuffer{};
};

#endif