#ifndef NST_ILI9341_DISPLAY_HPP_
#define NST_ILI9341_DISPLAY_HPP_

#include "Adafruit_ILI9341.h"
#include "nst/pin.hpp"
#include <array>
#include <cstdint>
#include <nst/hardware/ui/color.hpp>
#include <string_view>

namespace nst::teensy {

class TFT {
public:
  static constexpr std::size_t width = 320;
  static constexpr std::size_t height = 240;

public:
  TFT(Pin cs_pin, Pin dc_pin);

  void render_frame();

  void clear() { current_frame().fillScreen(0); }

  void set_frame_available() { m_frame_available = true; }

  [[nodiscard]] GFXcanvas16 &current_frame() {
    return *m_double_frame_buffer[0];
  }

  [[nodiscard]] Adafruit_ILI9341 &tft() { return m_tft; }

private:
  Adafruit_ILI9341 m_tft;

  GFXcanvas16 m_buffer1{width, height};
  GFXcanvas16 m_buffer2{width, height};
  std::array<GFXcanvas16 *, 2> m_double_frame_buffer{&m_buffer1, &m_buffer2};

  bool m_frame_available{false};
};

class ScreenElement {
public:
  ScreenElement(std::uint16_t x, std::uint16_t y, std::uint16_t width,
                std::uint16_t height, GFXcanvas16 &frame)
      : m_frame(frame), m_x(x), m_y(y), m_width(width), m_height(height) {}

  explicit ScreenElement(GFXcanvas16 &frame)
      : m_frame(frame), m_x(0), m_y(0), m_width(TFT::width),
        m_height(TFT::height) {}

  virtual ~ScreenElement() = default;

  virtual void draw() const = 0;

  [[nodiscard]] virtual std::uint16_t x() const { return m_x; }
  [[nodiscard]] virtual std::uint16_t y() const { return m_y; }

  [[nodiscard]] virtual std::uint16_t width() const { return m_width; }
  [[nodiscard]] virtual std::uint16_t height() const { return m_height; }

  virtual void set_x(std::uint16_t x) { m_x = x; }
  virtual void set_y(std::uint16_t y) { m_x = y; }

  virtual void set_width(std::uint16_t width) { m_width = width; }
  virtual void set_height(std::uint16_t m_height) { m_height = m_height; }

  virtual void center_y() { m_x = (TFT::width - m_width) / 2; }
  virtual void center_y() { m_y = (TFT::height - m_height) / 2; }

  virtual void center_x(ScreenElement &other) {
    m_x = other.x() + (other.width() - m_width) / 2;
  }
  virtual void center_y(ScreenElement &other) {
    m_y = other.y() + (other.height() - m_height) / 2;
  }

  virtual void center_x(std::uint16_t x, std::uint16_t width) {
    m_x = x + (width - m_width) / 2;
  }
  virtual void center_y(std::uint16_t y, std::uint16_t m_height) {
    m_y = y + (m_height - m_height) / 2;
  }

  void draw_boundary() const {
    m_frame.drawRect(m_x, m_y, m_width, m_height, ILI9341_GREEN);
    m_frame.drawCircle(m_x, m_y, 3, ILI9341_WHITE);
  }

  [[nodiscard]] constexpr static std::uint16_t
  centered_x(std::uint16_t width, std::uint16_t parent_x,
             std::uint16_t parent_width) {
    return parent_x + (parent_width - width) / 2;
  }

  [[nodiscard]] constexpr static std::uint16_t
  centered_y(std::uint16_t height, std::uint16_t parent_y,
             std::uint16_t parent_height) {
    return parent_y + (parent_height - height) / 2;
  }

protected:
  GFXcanvas16 &m_frame;
  std::uint16_t m_x, m_y, m_width, m_height;
};

class Text : public ScreenElement {
public:
  Text(std::uint16_t x, std::uint16_t y, std::string_view text,
       std::uint32_t color, std::uint8_t font_size, GFXcanvas16 &frame)
      : ScreenElement(x, y, 0, 0, frame), m_text(text),
        m_color(nst::teensy::to_565_color(color)), m_font_size(font_size) {
    int16_t _x, _y; // discard results
    m_frame.setTextSize(m_font_size);
    m_frame.getTextBounds(m_text.data(), m_x, m_y, &_x, &_y, &m_width, &m_height);
    m_height = max_text_height();
  }

  void draw() const override {
    m_frame.setTextColor(m_color);
    m_frame.setTextSize(m_font_size);
    m_frame.setCursor(m_x - m_font_size, m_y + m_height - m_font_size * 3);
    m_frame.println(m_text.data());
  }

  static void print(std::uint16_t x, std::uint16_t y, std::string_view text,
                    std::uint32_t color, std::uint8_t fontSize,
                    GFXcanvas16 &frame) {
    frame.setTextColor(nst::teensy::to_565_color(color));
    frame.setTextSize(fontSize);
    frame.setCursor(x, y);
    frame.print(text.data());
  }

  [[nodiscard]] std::uint16_t color() const { return m_color; }

private:
  [[nodiscard]] std::uint16_t max_text_height() const {
    int16_t x, y;
    std::uint16_t width, height;
    // characters with largest height range to make text boxes more consistent
    m_frame.getTextBounds("gT", m_x, m_y, &x, &y, &width, &height);
    return height;
  }

private:
  std::string_view m_text;
  std::uint16_t m_color;
  std::uint8_t m_font_size;
};

class Bitmap : public ScreenElement {
public:
  Bitmap(std::uint16_t x, std::uint16_t y, std::uint16_t width,
         std::uint16_t height, const std::uint16_t *bitmap565,
         GFXcanvas16 &frame)
      : ScreenElement(x, y, width, height, frame), data_(bitmap565) {}

  virtual ~Bitmap() = default;

  void draw() const override {
    m_frame.drawRGBBitmap(m_x, m_y, data_, m_width, m_height);
  }

private:
  const std::uint16_t *data_;
};

} // namespace nst::teensy

#endif