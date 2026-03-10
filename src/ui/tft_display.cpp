#include "tft_display.hpp"
#include <cstring>
#include "Fonts/FreeSansBoldOblique9pt7b.h"

TFT::TFT() 
{ 
  m_TFT.begin();
  m_TFT.setRotation(3);
  m_TFT.fillScreen(0);
  m_FrameBuffer1.setFont(&FreeSansBoldOblique9pt7b);
  m_FrameBuffer2.setFont(&FreeSansBoldOblique9pt7b);
};
 
void TFT::renderFrame() 
{
  if (!m_FrameAvailable) { return; }

  auto* previousBuffer = m_DoubleFrameBuffer[1];
  auto* currentBuffer = m_DoubleFrameBuffer[0];

  auto* previousFrameData = previousBuffer->getBuffer();
  auto* currentFrameData = currentBuffer->getBuffer();

  constexpr static auto rowSizeBytes = sizeof(std::uint16_t) * TFT::width;

  for (std::size_t row{}; row < TFT::height; row++)
  {
    const auto rowOffset = TFT::width * row;
    const auto currentRow = currentFrameData + rowOffset;
    const auto previousRow = previousFrameData + rowOffset;

    const auto rowDiff = std::memcmp(currentRow, previousRow, rowSizeBytes);

    if (rowDiff) 
    { 
      m_TFT.drawRGBBitmap(0, row, currentRow, TFT::width, 1);
      std::memcpy(previousRow, currentRow, rowSizeBytes);
    }
  }

  auto* tmp = m_DoubleFrameBuffer[0];
  m_DoubleFrameBuffer[0] = m_DoubleFrameBuffer[1];
  m_DoubleFrameBuffer[1] = tmp;

  m_FrameAvailable = false;
}