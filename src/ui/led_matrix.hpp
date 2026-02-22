#ifndef led_matrix_hpp_
#define led_matrix_hpp_

#include "led_frame.hpp"
#include "grid.hpp"
#include "color.hpp"
#include "trellis.hpp"
#include <cstdint>
#include <algorithm>

class LEDMatrixDisplay
{
public:
  explicit LEDMatrixDisplay(Trellis& trellis) 
  : m_Trellis(trellis.trellis()) 
  {
    m_FrameBuffer1.clear();
    m_FrameBuffer2.clear();
  }

  [[nodiscard]] LEDFrame& currentFrame() { return *m_DoubleFrameBuffer[0]; }

  void clear() { currentFrame().clear(); }

  void setFrameAvailable(bool available) { m_FrameAvailable = available; }

  void drawPixel(sndbx::grid::Position pos, std::uint32_t color) 
  { 
    currentFrame().drawPixel(pos, color);
    m_FrameAvailable = true;
  }

  void drawPixel(int row, int col, std::uint32_t color) 
  { 
    currentFrame().drawPixel(row, col, color);
    m_FrameAvailable = true;
  }

  void renderFrame() 
  {
    if (!m_FrameAvailable) { return; }

    auto& previousFrame = *m_DoubleFrameBuffer[1];
    auto& currentFrame = *m_DoubleFrameBuffer[0];

    for (std::size_t px{}; px < LEDFrame::size; px++)
    {
      auto currentPixel = currentFrame[px];
      if (currentPixel == previousFrame[px]) { continue; }

      const auto pixelColor = sndbx::color::changeBrightness(currentPixel, m_Brightness);
      m_Trellis.setPixelColor(px, pixelColor);

      previousFrame[px] = currentPixel;
    }

    m_Trellis.show();
    swapFrames();
    m_FrameAvailable = false;
  }

  void brightness(float brightness) { m_Brightness = brightness; }

private:
  void swapFrames()
  {
    auto* tmp = m_DoubleFrameBuffer[0];
    m_DoubleFrameBuffer[0] = m_DoubleFrameBuffer[1];
    m_DoubleFrameBuffer[1] = tmp;
  }

private:
  Adafruit_MultiTrellis& m_Trellis;
  LEDFrame m_FrameBuffer1;
  LEDFrame m_FrameBuffer2;
  std::array<LEDFrame*, 2> m_DoubleFrameBuffer{&m_FrameBuffer1, &m_FrameBuffer2};
  bool m_FrameAvailable{false};
  float m_Brightness{0.5f};
};

#endif