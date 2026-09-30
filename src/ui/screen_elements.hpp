#ifndef SANDBOX_SCREEN_ELEMENTS_HPP_
#define SANDBOX_SCREEN_ELEMENTS_HPP_

#include <string_view>

#include "assets/sandbox_logo_bitmap.hpp"
#include "config/config.hpp"
#include "nst/hardware/ILI9341_display.hpp"
#include "ui/color.hpp"
#include <nst/inplace_vector.hpp>

/**
 * @brief Representing a screen display element. Designed to be stack objects,
 *        dimensions must be known at construction time to use centering
 */
namespace sndbx {

class UIElement {
public:
  UIElement(std::uint16_t x, std::uint16_t y, std::uint16_t width,
            std::uint16_t height, GFXcanvas16 &frame)
      : x_{x}, y_{y}, width_{width}, height_{height}, frame_{frame} {}

  explicit UIElement(GFXcanvas16 &frame)
      : x_{0}, y_{0}, width_(limits::screen_width_px),
        height_(limits::screen_height_px), frame_{frame} {}

  [[nodiscard]] auto x() const { return x_; }
  [[nodiscard]] auto y() const { return y_; }

  [[nodiscard]] auto width() const { return width_; }
  [[nodiscard]] auto height() const { return height_; }

  void set_x(std::uint16_t x) { x_ = x; }
  void set_y(std::uint16_t y) { y_ = y; }

  void set_width(std::uint16_t width) { width_ = width; }
  void set_height(std::uint16_t height) { height_ = height; }

  void center_x() { x_ = (limits::screen_width_px - width_) / 2; }
  void center_y() { y_ = (limits::screen_height_px - height_) / 2; }

  void center_x(UIElement &other) {
    x_ = other.x() + (other.width() - width_) / 2;
  }
  void center_y(UIElement &other) {
    y_ = other.y() + (other.height() - height_) / 2;
  }

  void center_x(std::uint16_t x, std::uint16_t width) {
    x_ = x + (width - width_) / 2;
  }
  void center_y(std::uint16_t y, std::uint16_t height) {
    y_ = y + (height - height_) / 2;
  }

  void draw_bounds() const {
    frame_.drawRect(x_, y_, width_, height_, ILI9341_GREEN);
    frame_.drawCircle(x_, y_, 3, ILI9341_WHITE);
  }

  [[nodiscard]] static constexpr auto centered_x(std::uint16_t width,
                                                 std::uint16_t parent_x,
                                                 std::uint16_t parent_w) {
    return parent_x + (parent_w - width) / 2;
  }

  [[nodiscard]] static constexpr auto centered_y(std::uint16_t height,
                                                 std::uint16_t parent_y,
                                                 std::uint16_t parent_h) {
    return parent_y + (parent_h - height) / 2;
  }

protected:
  std::uint16_t x_;
  std::uint16_t y_;
  std::uint16_t width_;
  std::uint16_t height_;
  GFXcanvas16 &frame_;
};

//===============================================================================================
//  General colored sizable text
//===============================================================================================
class Text : public UIElement {
public:
  Text(std::string_view text, std::uint16_t x, std::uint16_t y,
       std::uint32_t color, std::uint8_t pt, GFXcanvas16 &frame)
      : UIElement{x, y, 0, 0, frame}, text_{text},
        color_{sndbx::color::to_565(color)}, font_size_{pt} {
    int16_t X, Y; // discard results
    frame.setTextSize(font_size_);
    frame.getTextBounds(text_.data(), x_, y_, &X, &Y, &width_, &height_);
    height_ = max_text_height();
  }

  void draw() const {
    frame_.setTextColor(color_);
    frame_.setTextSize(font_size_);
    frame_.setCursor(x_ - font_size_, y_ + height_ - font_size_ * 3);
    frame_.println(text_.data());
  }

  [[nodiscard]] std::uint16_t color() const { return color_; }

  static void print(std::string_view text, std::uint16_t x, std::uint16_t y,
                    std::uint32_t color, std::uint8_t pt, GFXcanvas16 &frame) {
    frame.setTextColor(sndbx::color::to_565(color));
    frame.setTextSize(pt);
    frame.setCursor(x, y);
    frame.print(text.data());
  }

private:
  std::uint16_t max_text_height() const {
    int16_t x, y;
    std::uint16_t width, height;
    // characters with largest height range, make text boxes more consistent
    frame_.getTextBounds("gT", x_, y_, &x, &y, &width, &height);
    return height;
  }

  std::string_view text_;
  std::uint16_t color_;
  std::uint8_t font_size_;
};

//===============================================================================================
// Error text display
//===============================================================================================

class ErrorDisplay : public UIElement {
public:
  ErrorDisplay(std::string_view text, GFXcanvas16 &frame)
      : UIElement{frame}, text_{text, 0, 120, 0xFFFFFF, 1, frame} {
    text_.center_x();
  }

  void draw() const {
    Text::print("error", 115, 80, 0xFF0000, 2, frame_);
    text_.draw();
  }

private:
  Text text_;
};
//===============================================================================================
// 565 color bitmap
//===============================================================================================
class Bitmap : public UIElement {
public:
  Bitmap(std::uint16_t x, std::uint16_t y, std::uint16_t width,
         std::uint16_t height, const std::uint16_t *bitmap565,
         GFXcanvas16 &frame)
      : UIElement{x, y, width, height, frame}, data_{bitmap565} {}

  void draw() const { frame_.drawRGBBitmap(x_, y_, data_, width_, height_); }

private:
  const std::uint16_t *data_;
};

//===============================================================================================
// Startup splash screen bitmap
//===============================================================================================

struct SplashScreen {
  SplashScreen(GFXcanvas16 &frame)
      : bitmap(0, 0, limits::screen_width_px, limits::screen_height_px,
               SANDBOX_LOGO_BITMAP.data(), frame) {}
  void draw() const { bitmap.draw(); }
  Bitmap bitmap;
};

//===============================================================================================
//  Labeled knob with variable turn amount
//===============================================================================================
class Knob : public UIElement {
public:
  static constexpr std::uint8_t radius = 25;
  static constexpr std::uint8_t width = radius * 2 + 1;

public:
  Knob(std::uint16_t x, std::uint16_t y, float percent_turned,
       std::string_view label, GFXcanvas16 &frame)
      : UIElement(x, y, Knob::width, Knob::width, frame),
        percent_turned_(percent_turned), label_(label) {}

  void draw() const {
    constexpr static std::uint16_t label_color = ILI9341_LIGHTGREY;
    constexpr static std::uint16_t border_color = ILI9341_WHITE;
    constexpr static std::uint16_t inner_color = ILI9341_DARKGREY;
    constexpr static std::uint16_t mark_color = ILI9341_WHITE;

    // Knob
    frame_.drawCircle(x_, y_, Knob::radius, border_color);
    frame_.drawCircle(x_, y_, 10, inner_color);

    constexpr static int min_rotation = 120;
    constexpr static int max_rotation = 420;
    constexpr static int mark_radius = 6;
    constexpr static float mark_scale = 0.9f;

    const int degrees_turned =
        min_rotation + (percent_turned_ * (max_rotation - min_rotation));
    const auto angle = degrees_turned % 360 * (M_PI / 180.0f);
    const auto offset = (Knob::radius - mark_radius) * mark_scale;

    const std::int16_t mark_x = x_ + std::cos(angle) * offset;
    const std::int16_t mark_y = y_ + std::sin(angle) * offset;

    frame_.fillCircle(mark_x, mark_y, mark_radius, mark_color);

    // Knob label

    int16_t text_x, text_y;
    std::uint16_t text_width, text_height;
    frame_.setTextSize(1);
    frame_.getTextBounds(label_.data(), 0, y_, &text_x, &text_y, &text_width,
                         &text_height);

    constexpr static int x_offset = 2;
    constexpr static int y_offset = 20;
    text_x = (x_ - Knob::radius) + ((width_ - text_width) / 2) - x_offset;
    text_y = y_ + Knob::radius + y_offset;

    frame_.setCursor(text_x, text_y);
    frame_.setTextColor(label_color);
    frame_.print(label_.data());
  }

public:
  const float percent_turned_;
  std::string_view label_;
};

class PortsDisplay : public UIElement {
public:
  PortsDisplay(std::uint16_t x, std::uint16_t y,
               const nst::vector_8U<std::string_view> &labels,
               const nst::vector_8U<std::uint32_t> &colors, GFXcanvas16 &frame)
      : UIElement(x, y, 0, square_width, frame), labels_(labels),
        colors_(colors) {
    const auto num_ports = labels_.size();
    width_ = (num_ports == 0)
                 ? 0
                 : square_width * num_ports + spacing_px * (num_ports - 1);
  }

  void draw() const {
    if (labels_.size() == 0) {
      return;
    }

    for (std::size_t i{}; i < std::min(labels_.size(), colors_.size()); ++i) {
      int x = x_ + i * (square_width + spacing_px);
      const auto color = colors_[i];
      std::uint32_t text_color{};

      frame_.drawRoundRect(x, y_, square_width, square_width, square_radius - 1,
                           ILI9341_DARKGREY);

      if (color != 0) {
        frame_.fillRoundRect(x + 1, y_ + 1, square_width - 2, square_width - 2,
                             square_radius, sndbx::color::to_565(color));
      } else {
        text_color = 0x606060;
      }

      Text label(labels_[i].data(), 0, 0, text_color, 1, frame_);
      label.center_x(x, square_width);
      label.center_y(y_, square_width);
      label.draw();
    }
  }

private:
  constexpr static auto square_width = 26;
  constexpr static auto square_radius = 6;
  constexpr static auto spacing_px = 7;
  constexpr static auto text_x_offset = 0;
  constexpr static auto text_y_offset = 1;
  const nst::vector_8U<std::string_view> &labels_;
  const nst::vector_8U<std::uint32_t> &colors_;
};

class ModuleDisplay : public UIElement {
public:
  ModuleDisplay(std::string_view name, std::uint32_t color,
                const nst::vector_4U<std::string_view> &ctrl_labels,
                const nst::vector_4U<float> &ctrl_vals,
                const nst::vector_8U<std::string_view> &in_names,
                const nst::vector_8U<std::uint32_t> &in_colors,
                const nst::vector_8U<std::string_view> &out_names,
                const nst::vector_8U<std::uint32_t> &out_colors,
                GFXcanvas16 &frame)

      : UIElement(frame), name_(name, 0, 65, color, name_size, frame),
        color_(sndbx::color::to_565(color)), ctrl_labels_(ctrl_labels),
        ctrl_vals_(ctrl_vals), inputs_(30, 25, in_names, in_colors, frame),
        outputs_(30, 110, out_names, out_colors, frame) {
    name_.center_x();
  }

  void draw() const {
    frame_.drawRoundRect(name_.x() - 2, name_.y() - 2, name_.width() + 4,
                         name_.height() + 2, 3, color_);
    name_.draw();
    inputs_.draw();
    outputs_.draw();
    Text::print("->", 10, 39, 0x606060, 1, frame_);
    Text::print("<-", 10, 126, 0x606060, 1, frame_);
    draw_knobs();
  }

private:
  void draw_knobs() const {
    constexpr static std::uint16_t knobs_y = 175;
    constexpr static auto spacing_px = 24;

    for (std::size_t i{}; i < 4; ++i) {
      const std::uint16_t knobX =
          Knob::radius + spacing_px + (Knob::radius + spacing_px * 2) * i;

      if (i < ctrl_labels_.size()) {
        Knob{knobX, knobs_y, ctrl_vals_[i], ctrl_labels_[i], frame_}.draw();
      } else {
        frame_.drawCircle(knobX, knobs_y, Knob::radius * 0.85,
                          ILI9341_DARKGREY);
      }
    }
  }

public:
  static void draw_knob(float value, std::string_view label, std::size_t index,
                        GFXcanvas16 &frame) {
    constexpr static std::uint16_t knobs_y = 175;
    constexpr static auto spacing_px = 24;

    const std::uint16_t knobX =
        Knob::radius + spacing_px + (Knob::radius + spacing_px * 2) * index;

    Knob{knobX, knobs_y, value, label, frame}.draw();
  }

  static void draw_empty_knob(std::uint16_t x, std::uint16_t y,
                              GFXcanvas16 &frame) {
    frame.drawCircle(x, y, Knob::radius * 0.85, ILI9341_DARKGREY);
  }

private:
  constexpr static std::uint8_t name_size = 2;
  Text name_;
  const std::uint16_t color_;
  const nst::vector_4U<std::string_view> &ctrl_labels_;
  const nst::vector_4U<float> &ctrl_vals_;
  PortsDisplay inputs_;
  PortsDisplay outputs_;
};

class BankDisplayPage : public UIElement {
public:
  BankDisplayPage(std::string_view name, std::string_view description,
                  std::uint32_t color, GFXcanvas16 &frame)
      : UIElement(frame),
        module_name_(name, x(), module_name_y, color, module_name_size, frame),
        description_(description, x(), module_name_.y() + spacing_px, 0x606060,
                     description_size, frame) {
    module_name_.center_x();
    description_.center_x();
  }

  void draw() const {
    module_name_.draw();
    description_.draw();

    Text tip{"Scroll with knob 1.", 50, 200, 0x606060, 1, frame_};
    tip.center_x();
    tip.draw();
  }

  static constexpr std::uint8_t module_name_size = 3;
  static constexpr std::uint8_t description_size = 1;

  static constexpr std::uint8_t module_name_y = 70;
  static constexpr std::uint8_t spacing_px = 60;

private:
  Text module_name_;
  Text description_;
};

inline void draw_down_arrow(std::uint16_t start_x, std::uint16_t start_y,
                            std::uint16_t end_x, std::uint16_t end_y,
                            std::uint16_t color565, GFXcanvas16 &frame) {
  frame.drawFastVLine(start_x, start_y, end_y - start_y, color565);
  frame.drawFastVLine(start_x + 1, start_y, end_y - start_y, color565);
  frame.drawFastVLine(start_x - 1, start_y, end_y - start_y, color565);

  frame.setTextSize(1);
  frame.setTextColor(color565);
  frame.setCursor(end_x - 7, end_y - 2);
  frame.print("V");
}

class PatchDisplayPage : public UIElement {
public:
  PatchDisplayPage(std::string_view src_name, std::string_view dst_name,
                   std::string_view src_port_name,
                   std::string_view dst_port_name, std::uint32_t src_color,
                   std::uint32_t dst_color, GFXcanvas16 &frame)
      : UIElement(frame), src_name_(src_name, x(), 50, src_color, 2, frame),
        dst_name_(dst_name, x(), 150, dst_color, 2, frame),
        src_port_name_(src_port_name, 15, y(), src_color, 1, frame),
        dst_port_name_(dst_port_name, 15, y(), dst_color, 1, frame) {
    src_name_.center_x();
    dst_name_.center_x();
    src_port_name_.center_y(src_name_);
    dst_port_name_.center_x(src_port_name_);
    dst_port_name_.center_y(dst_name_);
  }

  void draw() const {
    src_name_.draw();
    dst_name_.draw();
    src_port_name_.draw();
    dst_port_name_.draw();

    const auto src_port_x = src_port_name_.x();
    const auto src_port_y = src_port_name_.y();
    const auto src_port_width = src_port_name_.width();
    const auto src_port_height = src_port_name_.height();
    const auto src_rect_x =
        UIElement::centered_x(square_width, src_port_x, src_port_width);
    const auto src_rect_y =
        UIElement::centered_y(square_width, src_port_y, src_port_height);

    const auto dst_port_x = dst_port_name_.x();
    const auto dst_port_y = dst_port_name_.y();
    const auto dst_port_width = dst_port_name_.width();
    const auto dst_port_height = dst_port_name_.height();
    const auto dst_rect_x =
        UIElement::centered_x(square_width, dst_port_x, dst_port_width);
    const auto dst_rect_y =
        UIElement::centered_y(square_width, dst_port_y, dst_port_height);

    frame_.drawRoundRect(src_rect_x, src_rect_y, square_width, square_width, 5,
                         src_name_.color());
    frame_.drawRoundRect(dst_rect_x, dst_rect_y, square_width, square_width, 5,
                         dst_name_.color());

    const auto arrow_x1 = src_port_x + (src_port_width / 2);
    const auto arrow_y1 = src_port_y + src_port_height + 5;
    const auto arrow_x2 = dst_port_x + (dst_port_width / 2);
    const auto arrow_y2 = dst_port_y - 5;

    draw_down_arrow(arrow_x1, arrow_y1, arrow_x2, arrow_y2, src_name_.color(),
                    frame_);
  }

private:
  constexpr static auto square_width = 26U;
  Text src_name_;
  Text dst_name_;
  Text src_port_name_;
  Text dst_port_name_;
};

class OscilloscopeFrame : public UIElement {
  static constexpr std::size_t bufferSize = 1024;
  using SampleBuffer = std::array<float, bufferSize>;

public:
  OscilloscopeFrame(const SampleBuffer &buffer, GFXcanvas16 &frame)
      : UIElement(frame), buffer_(buffer) {}

  void draw() const {
    frame_.fillScreen(0);

    frame_.drawRoundRect(window_x, window_y, window_width, window_height, 20,
                         border_color);
    frame_.drawFastHLine(window_x, window_y + window_height / 2, window_width,
                         zero_line_color);

    Text title{"scope", 0, 6, ILI9341_CYAN, 1, frame_};
    title.center_x();
    title.draw();

    uint16_t color;
    int prev_x = 0;
    int prev_y = 0;

    for (std::size_t i{}; i < buffer_.size(); i += downsample) {
      const auto sample = buffer_[i];
      const auto percent_drawn = static_cast<float>(i) / (buffer_.size() - 1);

      const auto x =
          (window_x + 1) + static_cast<int>(percent_drawn * (window_width - 1));
      const auto y = (window_height / 2) -
                     (window_height / 2) * (sample * waveform_scale) + window_y;

      if (i == 0) {
        prev_x = x;
        prev_y = y;
        continue;
      }

      if (prev_x == x && prev_y == y) {
        continue;
      }

      color = (sample <= -1.0f || sample >= 1.0f) ? clipping_color : safe_color;

      frame_.drawLine(prev_x, prev_y, x, y, color);
      prev_x = x;
      prev_y = y;
    }
  }

private:
  constexpr static auto downsample = 8U;
  constexpr static float waveform_scale = 0.7f;
  constexpr static float window_scale = 0.85f;

  constexpr static auto window_width = limits::screen_width_px * window_scale;
  constexpr static auto window_height = limits::screen_height_px * window_scale;
  constexpr static auto window_x =
      centered_x(window_width, 0, limits::screen_width_px);
  constexpr static auto window_y =
      centered_y(window_height, 0, limits::screen_height_px) + 7;

  constexpr static auto zero_line_color = ILI9341_DARKGREY;
  constexpr static auto border_color = ILI9341_DARKCYAN;
  constexpr static auto safe_color = ILI9341_GREEN;
  constexpr static auto clipping_color = ILI9341_RED;

  const SampleBuffer &buffer_;
};

class KeyboardElement : public UIElement {
  constexpr static auto key_width = 26;
  constexpr static auto key_radius = 6;
  constexpr static auto key_spacing = 7;

public:
  constexpr static auto text_x_offset = -15;
  constexpr static auto text_y_offset = 1;
  constexpr static auto text_size = 2U;

  constexpr static auto num_keys = 40U;
  constexpr static auto num_rows = num_keys / 7;

  static constexpr std::array<std::string_view, num_keys> key_labels
      FLASHMEM = {"a", "b", "c", "d", "e", "f", "g", "h",   "i", "j",
                  "k", "l", "m", "n", "o", "p", "q", "r",   "s", "t",
                  "u", "v", "w", "x", "y", "z", "0", "1",   "2", "3",
                  "4", "5", "6", "7", "8", "9", "-", "del", ">", "X"};

public:
  KeyboardElement(std::string_view current_text, GFXcanvas16 &frame)
      : UIElement(0, 60, 0, 0, frame),
        cur_text_{current_text, 0, 0, ILI9341_LIGHTGREY, 2, frame} {
    width_ = (key_width + key_spacing) * 8 - key_width;
    height_ = (key_width + key_spacing) * 5 + cur_text_.height() - 15;
    center_x();
    cur_text_.center_x();
    cur_text_.set_x(y_ - cur_text_.height() + 1);
  }

  void draw() const {
    frame_.fillScreen(0);
    cur_text_.draw();

    for (std::size_t key_idx{}; key_idx < num_keys; key_idx++) {
      std::uint16_t key_x = x_ + (key_idx % 8) * (key_width + key_spacing);
      std::uint16_t key_y = y_ + (key_idx / 8) * (key_width + key_spacing);

      auto color = (key_idx % 2 == 0) ? ILI9341_DARKCYAN : ILI9341_CYAN;

      if (key_idx >= digits_start && key_idx < backspace_idx) {
        color = ILI9341_LIGHTGREY;
      } else if (key_idx == backspace_idx) {
        color = ILI9341_DARKGREY;
      } else if (key_idx == enter_idx) {
        color = ILI9341_GREEN;
      } else if (key_idx == escape_idx) {
        color = ILI9341_RED;
      }

      frame_.fillRoundRect(key_x, key_y, key_width, key_width, key_radius,
                           color);

      Text key_label{key_labels[key_idx], 0, 0, ILI9341_BLACK, 1, frame_};
      key_label.center_x(key_width, key_x);
      key_label.center_y(key_width, key_y);
      key_label.draw();
    }
  }

public:
  constexpr static auto digits_start = 26U;
  constexpr static auto backspace_idx = 37U;
  constexpr static auto enter_idx = 38U;
  constexpr static auto escape_idx = 39U;
  Text cur_text_;
};

} // namespace sndbx

#endif