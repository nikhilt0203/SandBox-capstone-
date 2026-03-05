#ifndef SANDBOX_TFT_DISPLAY_HPP_
#define SANDBOX_TFT_DISPLAY_HPP_

#include "pinouts.hpp"
#include "Adafruit_ILI9341.h"
#include <cstdint>

class TFT
{
public:
  TFT();

  void renderFrame(); 

  void clear() { currentFrame().fillScreen(0); }

  void setFrameAvailable(bool available) { m_FrameAvailable = available; }

  [[nodiscard]] GFXcanvas16& currentFrame() { return *m_DoubleFrameBuffer[0]; }

  [[nodiscard]] Adafruit_ILI9341& tft() { return m_TFT; }

public:
  static constexpr std::size_t width = 320U;
  static constexpr std::size_t height = 240U;

private:
  Adafruit_ILI9341 m_TFT{TFT_CS_PIN, TFT_DC_PIN};

  GFXcanvas16 m_FrameBuffer1{width, height};
  GFXcanvas16 m_FrameBuffer2{width, height};

  std::array<GFXcanvas16*, 2> m_DoubleFrameBuffer{&m_FrameBuffer1, &m_FrameBuffer2};

  bool m_FrameAvailable{};
};

#endif