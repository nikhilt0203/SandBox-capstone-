#ifndef SANDBOX_SCREEN_ELEMENTS_HPP_
#define SANDBOX_SCREEN_ELEMENTS_HPP_

#include "Adafruit_ILI9341.h"
#include "ui/tft_display.hpp"
#include <string_view>
#include "ui/color.hpp"
#include "core/fixed_vector.hpp"


/**
 * @brief Representing a screen display element. Designed to be stack objects,
 *        dimensions must be known at construction time to use centering
 */
class ScreenElement
{
public:
  ScreenElement(std::uint16_t x, std::uint16_t y, std::uint16_t width, std::uint16_t height, GFXcanvas16& frame) 
  : m_Frame(frame),
    m_X(x),
    m_Y(y),
    m_Width(width),
    m_Height(height)
  {}

  explicit ScreenElement(GFXcanvas16& frame) 
  : m_Frame(frame),
    m_X(0),
    m_Y(0),
    m_Width(TFT::width),
    m_Height(TFT::height)
  {}

  virtual ~ScreenElement() = default;

  virtual void draw() const = 0;

  [[nodiscard]] virtual std::uint16_t x() const { return m_X; }
  [[nodiscard]] virtual std::uint16_t y() const { return m_Y; }

  [[nodiscard]] virtual std::uint16_t width() const { return m_Width; }
  [[nodiscard]] virtual std::uint16_t height() const { return m_Height; }

  virtual void setX(std::uint16_t x) { m_X = x; }
  virtual void setY(std::uint16_t y) { m_Y = y; }

  virtual void setWidth(std::uint16_t width) { m_Width = width; }
  virtual void setHeight(std::uint16_t height) { m_Height = height; }

  virtual void centerX() { m_X = (TFT::width - m_Width) / 2; }
  virtual void centerY() { m_Y = (TFT::height - m_Height) / 2; }

  virtual void centerX(ScreenElement& other) { m_X = other.x() + (other.width() - m_Width) / 2; }
  virtual void centerY(ScreenElement& other) { m_Y = other.y() + (other.height() - m_Height) / 2; }

  virtual void centerX(std::uint16_t x, std::uint16_t width) { m_X = x + (width - m_Width) / 2; }
  virtual void centerY(std::uint16_t y, std::uint16_t height) { m_Y = y + (height - m_Height) / 2; }

  void drawBoundary() const
  { 
    m_Frame.drawRect(m_X, m_Y, m_Width, m_Height, ILI9341_GREEN);
    m_Frame.drawCircle(m_X, m_Y, 3, ILI9341_WHITE);
  }

  [[nodiscard]] constexpr static std::uint16_t centeredX(
                                               std::uint16_t width,
                                               std::uint16_t parentX, 
                                               std::uint16_t parentWidth) 
  { 
    return parentX + (parentWidth - width) / 2; 
  }

  [[nodiscard]] constexpr static std::uint16_t centeredY(std::uint16_t height, 
                                                         std::uint16_t parentY, 
                                                         std::uint16_t parentHeight) 
  { 
    return parentY + (parentHeight - height) / 2; 
  }

protected:
  GFXcanvas16& m_Frame;
  std::uint16_t m_X, m_Y, m_Width, m_Height;
};

//===============================================================================================
//  General colored sizable text
//===============================================================================================
class Text : public ScreenElement
{
public:
  Text(std::uint16_t x, 
       std::uint16_t y, 
       std::string_view text, 
       std::uint32_t color, 
       std::uint8_t fontSize,
       GFXcanvas16& frame)
  : ScreenElement(x, y, 0, 0, frame),
    m_Text(text),
    m_Color(sndbx::color::to565(color)),
    m_FontSize(fontSize)
  {
    int16_t X, Y; //discard results
    m_Frame.setTextSize(m_FontSize);
    m_Frame.getTextBounds(m_Text.data(), m_X, m_Y, &X, &Y, &m_Width, &m_Height);
    m_Height = getMaxTextHeight();
  }

  void draw() const override 
  {
    m_Frame.setTextColor(m_Color);
    m_Frame.setTextSize(m_FontSize);
    m_Frame.setCursor(m_X - m_FontSize, m_Y + m_Height - m_FontSize * 3);
    m_Frame.println(m_Text.data()); 
  }

  static void print(std::uint16_t x, 
                    std::uint16_t y,  
                    std::string_view text,  
                    std::uint32_t color, 
                    std::uint8_t fontSize,
                    GFXcanvas16& frame)
  {
    frame.setTextColor(sndbx::color::to565(color));
    frame.setTextSize(fontSize);
    frame.setCursor(x, y);
    frame.print(text.data()); 
  }

  [[nodiscard]] std::uint16_t color() const noexcept { return m_Color; }

private:
  [[nodiscard]] std::uint16_t getMaxTextHeight() const
  {
    int16_t x, y;
    std::uint16_t width, height;
    //characters with largest height range, make text boxes more consistent
    m_Frame.getTextBounds("gT", m_X, m_Y, &x, &y, &width, &height);
    return height;
  }

private:
  std::string_view m_Text;
  std::uint16_t m_Color;
  std::uint8_t m_FontSize;
};

//===============================================================================================
// Error text display
//===============================================================================================

class ErrorDisplay : public ScreenElement
{
public:
  ErrorDisplay(std::string_view text, GFXcanvas16& frame)
  : ScreenElement(frame),
    m_Text(0, 120, text, 0xFFFFFF, 1, frame)
  {
    m_Text.centerX();
  }

  void draw() const override 
  { 
    Text::print(115, 80, "error", 0xFF0000, 2, m_Frame);
    m_Text.draw();
  }

private:
  Text m_Text;
};
//===============================================================================================
// 565 color bitmap
//===============================================================================================
class Bitmap : public ScreenElement
{
public:
  Bitmap(std::uint16_t x, 
         std::uint16_t y, 
         std::uint16_t width, 
         std::uint16_t height, 
         const std::uint16_t* bitmap565,
         GFXcanvas16& frame)
  : ScreenElement(x, y, width, height, frame),
    m_BitmapData(bitmap565)
  {}

  virtual ~Bitmap() = default;

  void draw() const override { m_Frame.drawRGBBitmap(m_X, m_Y, m_BitmapData, m_Width, m_Height); }

private:
  const std::uint16_t* m_BitmapData;
};

//===============================================================================================
// Startup splash screen bitmap
//===============================================================================================

#include "sandbox_logo_bitmap.hpp"
class SplashScreen : public Bitmap
{
public:
  SplashScreen(GFXcanvas16& frame) 
  : Bitmap(0, 0, TFT::width, TFT::height, SANDBOX_LOGO_BITMAP.data(), frame) 
  {}
};

//===============================================================================================
//  Labeled knob with variable turn amount
//===============================================================================================
class Knob : public ScreenElement
{
public:
  static constexpr std::uint8_t radius = 25;
  static constexpr std::uint8_t width = radius * 2 + 1;

public:
  Knob(std::uint16_t x, 
       std::uint16_t y, 
       float percentTurned, 
       std::string_view label, 
       GFXcanvas16& frame)
  : ScreenElement(x, y, Knob::width, Knob::width, frame), 
    m_PercentTurned(percentTurned), 
    m_Label(label) 
  {}

  void draw() const override
  {
    constexpr static std::uint16_t labelColor = ILI9341_LIGHTGREY;
    constexpr static std::uint16_t outerCircleColor = ILI9341_WHITE;
    constexpr static std::uint16_t innerCircleColor = ILI9341_DARKGREY;
    constexpr static std::uint16_t indicatorColor = ILI9341_WHITE;

    //Knob
    m_Frame.drawCircle(m_X, m_Y, Knob::radius, outerCircleColor);
    m_Frame.drawCircle(m_X, m_Y, 10, innerCircleColor);

    constexpr static int minRotation = 120;
    constexpr static int maxRotation = 420;
    constexpr static int indicatorRadius = 6;
    constexpr static float indicatorScale = 0.9f;

    const int degreesTurned = minRotation + (m_PercentTurned * (maxRotation - minRotation));
    const auto angle = degreesTurned % 360 * (M_PI / 180.0f);
    const auto offset = (Knob::radius - indicatorRadius) * indicatorScale;

    const std::int16_t indicatorX = m_X + std::cos(angle) * offset;
    const std::int16_t indicatorY = m_Y + std::sin(angle) * offset;

    m_Frame.fillCircle(indicatorX, indicatorY, indicatorRadius, indicatorColor);

    //Knob label

    int16_t textX, textY;
    std::uint16_t textWidth, textHeight;
    m_Frame.setTextSize(1);
    m_Frame.getTextBounds(m_Label.data(), 0, m_Y, &textX, &textY, &textWidth, &textHeight);

    constexpr static int offsetX = 2;
    constexpr static int offsetY = 20;
    textX = (m_X - Knob::radius) + ((m_Width - textWidth) / 2) - offsetX;
    textY = m_Y + Knob::radius + offsetY;

    m_Frame.setCursor(textX, textY);
    m_Frame.setTextColor(labelColor);
    m_Frame.print(m_Label.data());
  }

public:
  const float m_PercentTurned;
  std::string_view m_Label;
};

class PortsDisplay : public ScreenElement
{
public:
  static constexpr int squareWidth = 26;
  static constexpr int squareRadius = 6;

  static constexpr int SPACING_PX = 7;

  static constexpr int TEXT_Y_OFFSET = 0;
  static constexpr int TEXT_X_OFFSET = 1;

public:
  PortsDisplay(std::uint16_t x, 
               std::uint16_t y, 
               const sndbx::vector_8U<std::string_view>& labels, 
               const sndbx::vector_8U<std::uint32_t>& colors, 
               GFXcanvas16& frame)
  : ScreenElement(x, y, 0, squareWidth, frame), 
    m_Labels(labels),
    m_Colors(colors)
  {
    const auto numPorts = m_Labels.size();
    
    m_Width = (numPorts == 0) ? 
      0 : squareWidth * numPorts + SPACING_PX * (numPorts - 1);
  }

  void draw() const override
  {
    const auto numConnections = m_Labels.size();

    if (numConnections == 0) { return; }

    for (std::size_t i{}; i < std::min(m_Labels.size(), m_Colors.size()); ++i)
    {
      int x = m_X + i * (squareWidth + SPACING_PX);
      const auto color = m_Colors.at(i);
      std::uint32_t textColor{};

      m_Frame.drawRoundRect(x, m_Y, squareWidth, squareWidth, squareRadius - 1, ILI9341_DARKGREY);

      if (color != 0)
      {
        m_Frame.fillRoundRect(x + 1, m_Y + 1, squareWidth - 2, squareWidth - 2, squareRadius, sndbx::color::to565(m_Colors.at(i)));
      }
      else
      {
        textColor = 0x606060;
      }

      Text portLabel(0, 0, m_Labels.at(i).data(), textColor, 1, m_Frame);
      portLabel.centerX(x, squareWidth);
      portLabel.centerY(m_Y, squareWidth);
      portLabel.draw();
    }
  }

public:
  const sndbx::vector_8U<std::string_view>& m_Labels;
  const sndbx::vector_8U<std::uint32_t>& m_Colors;
};

#include <string>
class ModuleDisplay : public ScreenElement
{
static constexpr std::uint8_t nameSize = 2;

public:
  ModuleDisplay(std::string_view name,
                std::uint32_t color,
                const sndbx::vector_4U<std::string_view>& controlLabels,
                const sndbx::vector_4U<float>& controlVals,
                const sndbx::vector_8U<std::string_view>& inputNames,
                const sndbx::vector_8U<std::uint32_t>& inputPortColors,
                const sndbx::vector_8U<std::string_view>& outputNames,
                const sndbx::vector_8U<std::uint32_t>& outputPortColors,
                GFXcanvas16& frame)

  : ScreenElement(frame),
    m_Name(0, 65, name, color, nameSize, frame),
    m_Color(sndbx::color::to565(color)),
    m_ControlLabels(controlLabels),
    m_ControlVals(controlVals),
    m_Inputs(30, 25, inputNames, inputPortColors, frame),
    m_Outputs(30, 110, outputNames, outputPortColors, frame)
  {
    m_Name.centerX();
  }

  void draw() const override
  {
    m_Frame.drawRoundRect(m_Name.x() - 2, m_Name.y() - 2, m_Name.width() + 4, m_Name.height() + 2, 3, m_Color);
    m_Name.draw();
    m_Inputs.draw();
    m_Outputs.draw();
    Text::print(10, 26, "->", 0x606060, 1, m_Frame);
    Text::print(10, 113, "<-", 0x606060, 1, m_Frame);
    drawKnobs();
  }

private:
  void drawKnobs() const
  {
    constexpr static std::uint16_t knobsY = 175;
    constexpr static auto spacingPx = 24;

    for (std::size_t i{}; i < 4; ++i)
    {
      const std::uint16_t knobX = Knob::radius + spacingPx + (Knob::radius + spacingPx * 2) * i;

      if (i < m_ControlLabels.size())
      {
        Knob{knobX, knobsY, m_ControlVals[i], m_ControlLabels[i], m_Frame}.draw();
      }
      else { m_Frame.drawCircle(knobX, knobsY, Knob::radius * 0.85, ILI9341_DARKGREY); }
    }
  }
  
private:
  Text m_Name;
  const std::uint16_t m_Color;

  const sndbx::vector_4U<std::string_view>& m_ControlLabels;
  const sndbx::vector_4U<float>& m_ControlVals;

  PortsDisplay m_Inputs;
  PortsDisplay m_Outputs;
};


class BankDisplayPage : public ScreenElement
{
public:
  BankDisplayPage(std::string_view name, std::string_view description, std::uint32_t color, GFXcanvas16& frame)
  : ScreenElement(frame), 
    m_ModuleNameText(m_X, moduleNameY, name, color, moduleNameSize, frame),
    m_DescriptionText(m_X, m_ModuleNameText.y() + textSpacing, description, 0x606060, descriptionSize, frame)
  {
    m_ModuleNameText.centerX();
    m_DescriptionText.centerX();
  }
 
  void draw() const override 
  {
    m_ModuleNameText.draw();
    m_DescriptionText.draw();

    Text tip(50, 200, "Scroll with knob 1.", 0x606060, 1, m_Frame);
    tip.centerX();
    tip.draw();
  }

public:
  static constexpr std::uint8_t moduleNameSize = 3;
  static constexpr std::uint8_t descriptionSize = 1;

  static constexpr std::uint8_t moduleNameY = 70;
  static constexpr std::uint8_t textSpacing = 60;

private:
  Text m_ModuleNameText;
  Text m_DescriptionText;
};

inline void drawDownArrow(std::uint16_t startX, 
                          std::uint16_t startY, 
                          std::uint16_t endX, 
                          std::uint16_t endY,
                          std::uint16_t color565,
                          GFXcanvas16& frame)
{
  frame.drawFastVLine(startX, startY, endY - startY, color565);
  frame.drawFastVLine(startX + 1, startY, endY - startY, color565);
  frame.drawFastVLine(startX - 1, startY, endY - startY, color565);

  frame.setTextSize(1);
  frame.setTextColor(color565);
  frame.setCursor(endX - 7, endY - 2);
  frame.print("V");
}

class PatchDisplayPage : public ScreenElement
{
public:
  PatchDisplayPage(
    std::string_view srcName, 
    std::string_view destName, 
    std::string_view srcPortName,
    std::string_view destPortName, 
    std::uint32_t srcColor,
    std::uint32_t destColor,
    GFXcanvas16& frame)
  : ScreenElement(frame), 
    m_SrcNameText(m_X, 50, srcName, srcColor, 2, frame),
    m_DestNameText(m_X, 150, destName, destColor, 2, frame),
    m_SrcPortText(15, m_Y, srcPortName, srcColor, 1, frame),
    m_DestPortText(15, m_Y, destPortName, destColor, 1, frame)
  {
    m_SrcNameText.centerX();
    m_DestNameText.centerX();
    m_SrcPortText.centerY(m_SrcNameText);
    m_DestPortText.centerX(m_SrcPortText);
    m_DestPortText.centerY(m_DestNameText);
  }
 
  void draw() const override 
  {
    m_SrcNameText.draw();
    m_DestNameText.draw();
    m_SrcPortText.draw();
    m_DestPortText.draw();

    constexpr static auto squareWidth = 26;

    const auto srcPortX = m_SrcPortText.x();
    const auto srcPortY = m_SrcPortText.y();
    const auto srcPortWidth = m_SrcPortText.width();
    const auto srcPortHeight= m_SrcPortText.height();

    const auto destPortX = m_DestPortText.x();
    const auto destPortY = m_DestPortText.y();
    const auto destPortWidth = m_DestPortText.width();
    const auto destPortHeight = m_DestPortText.height();

    const auto srcRectX = ScreenElement::centeredX(squareWidth, srcPortX, srcPortWidth);
    const auto srcRectY = ScreenElement::centeredY(squareWidth, srcPortY, srcPortHeight);
    const auto destRectX = ScreenElement::centeredX(squareWidth, destPortX, destPortWidth);
    const auto destRectY = ScreenElement::centeredY(squareWidth, destPortY, destPortHeight);

    const auto srcColor = m_SrcNameText.color();
    const auto destColor = m_DestNameText.color();

    m_Frame.drawRoundRect(srcRectX, srcRectY, squareWidth, squareWidth, 5, srcColor);
    m_Frame.drawRoundRect(destRectX, destRectY, squareWidth, squareWidth, 5, destColor);

    const auto arrowStartX = srcPortX + (srcPortWidth / 2);
    const auto arrowStartY = srcPortY + srcPortHeight + 5;
    const auto arrowEndX = destPortX + (destPortWidth / 2);
    const auto arrowEndY = destPortY - 5;

    drawDownArrow(arrowStartX, arrowStartY, arrowEndX, arrowEndY, srcColor, m_Frame);
  }

private:
  Text m_SrcNameText;
  Text m_DestNameText;
  Text m_SrcPortText;
  Text m_DestPortText;
};

class WaveformDisplayFrame : public ScreenElement
{
  static constexpr std::size_t bufferSize = 1024;
  using SampleBuffer = std::array<float, bufferSize>;

public:
  WaveformDisplayFrame(const SampleBuffer& sampleBuffer, GFXcanvas16& frame)
  : ScreenElement(frame), 
    m_SampleBuffer(sampleBuffer)
  {}

  void draw() const override
  {
    m_Frame.fillScreen(0);

    constexpr static float waveformScale = 0.7f;
    constexpr static float windowScale = 0.85f;

    constexpr static int windowWidth = TFT::width * windowScale;
    constexpr static int windowHeight = TFT::height * windowScale;
    constexpr static int windowX = centeredX(windowWidth, 0, TFT::width);
    constexpr static int windowY = centeredY(windowHeight, 0, TFT::height) + 7;

    constexpr static uint16_t zeroLineColor = ILI9341_DARKGREY;
    constexpr static uint16_t borderColor = ILI9341_DARKCYAN;

    m_Frame.drawRoundRect(windowX, windowY, windowWidth, windowHeight, 20, borderColor);
    m_Frame.drawFastHLine(windowX, windowY + windowHeight / 2, windowWidth, zeroLineColor);

    Text title(0, 6, "scope", ILI9341_CYAN, 1, m_Frame);
    title.centerX();
    title.draw();

    uint16_t color;
    int prevX = 0;
    int prevY = 0;

    constexpr static auto downsampleFactor = 6U;
    for (std::size_t i{}; i < m_SampleBuffer.size(); i += downsampleFactor)
    {
      const auto sampleValue = m_SampleBuffer[i];
      const auto percentDrawn = static_cast<float>(i) / (m_SampleBuffer.size() - 1);

      const auto x = (windowX + 1) + static_cast<int>(percentDrawn * (windowWidth - 1));
      const auto y = (windowHeight / 2) - (windowHeight / 2) * (sampleValue * waveformScale) + windowY;

      if (i == 0)
      {
        prevX = x;
        prevY = y;
        continue;
      }

      if (prevX == x && prevY == y) { continue; }

      constexpr static uint16_t waveformColor = ILI9341_GREEN;
      constexpr static uint16_t waveformClippingColor = ILI9341_RED;

      color = 
        (sampleValue <= -1.0f || sampleValue >= 1.0f) 
        ? waveformClippingColor 
        : waveformColor;

      m_Frame.drawLine(prevX, prevY, x, y, color);
      prevX = x;
      prevY = y;
    }
  }

private:
  const SampleBuffer& m_SampleBuffer;
};

#endif