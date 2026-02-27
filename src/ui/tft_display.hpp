#ifndef SANDBOX_TFT_DISPLAY_HPP_
#define SANDBOX_TFT_DISPLAY_HPP_

#include "pinouts.hpp"
#include "Adafruit_ILI9341.h"
#include <cstring>
#include <cstdint>
#include "Fonts/FreeSansBoldOblique9pt7b.h"

class TFT
{
public:
  static constexpr std::size_t width = 320U;
  static constexpr std::size_t height = 240U;

public:
  TFT() 
  { 
    m_TFT.begin();
    m_TFT.setRotation(3);
    m_TFT.fillScreen(0);
    m_FrameBuffer1.setFont(&FreeSansBoldOblique9pt7b);
    m_FrameBuffer2.setFont(&FreeSansBoldOblique9pt7b);
  };

  [[nodiscard]] GFXcanvas16& currentFrame() { return *m_DoubleFrameBuffer[0]; }

  void clear() { currentFrame().fillScreen(0); }

  void setFrameAvailable(bool available) { m_FrameAvailable = available; }

  void renderFrame() 
  {
    if (!m_FrameAvailable) { return; }

    auto* previousFrameData = m_DoubleFrameBuffer[1]->getBuffer();
    auto* currentFrameData = m_DoubleFrameBuffer[0]->getBuffer();

    constexpr static auto rowSizeBytes = sizeof(std::uint16_t) * TFT::width;

    for (std::size_t row{}; row < TFT::height; row++)
    {
      const auto rowOffset = TFT::width * row;
      const auto currentRow = currentFrameData + rowOffset;
      const auto previousRow = previousFrameData + rowOffset;

      auto rowDiff = std::memcmp(currentRow, previousRow, rowSizeBytes);
  
      if (rowDiff) 
      { 
        m_TFT.drawRGBBitmap(0, row, currentRow, TFT::width, 1);
        std::memcpy(previousRow, currentRow, rowSizeBytes);
      }
    }

    swapFrames();
    m_FrameAvailable = false;
  }

  [[nodiscard]] Adafruit_ILI9341& tft() { return m_TFT; }

private:
  void swapFrames()
  {
    auto* tmp = m_DoubleFrameBuffer[0];
    m_DoubleFrameBuffer[0] = m_DoubleFrameBuffer[1];
    m_DoubleFrameBuffer[1] = tmp;
  }

private:
  Adafruit_ILI9341 m_TFT{TFT_CS_PIN, TFT_DC_PIN};
  GFXcanvas16 m_FrameBuffer1{width, height};
  GFXcanvas16 m_FrameBuffer2{width, height};
  std::array<GFXcanvas16*, 2> m_DoubleFrameBuffer{&m_FrameBuffer1, &m_FrameBuffer2};
  bool m_FrameAvailable{};
};

#endif