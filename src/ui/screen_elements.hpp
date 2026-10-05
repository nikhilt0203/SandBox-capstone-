#ifndef SANDBOX_SCREEN_ELEMENTS_HPP_
#define SANDBOX_SCREEN_ELEMENTS_HPP_

#include <string_view>

#include "assets/sandbox_logo_bitmap.hpp"
#include "config/config.hpp"
#include "modules/module_display_info.hpp"
#include "nst/hardware/ILI9341_display.hpp"
#include "ui/ui_element.hpp"
#include "ui/ui_text.hpp"
#include <nst/inplace_vector.hpp>

namespace sndbx {

using ScreenElement =
    UIElement<GFXcanvas16, limits::screen_width_px, limits::screen_height_px>;

//===============================================================================================
//  General colored sizable text
//===============================================================================================
class Text : public ScreenElement {
  public:
	Text(GFXcanvas16 &frame, SizedColoredText data, std::uint16_t x,
	     std::uint16_t y)
	    : ScreenElement{frame, x, y, 0, 0}, text_{data.text.data()},
	      color_565_{to_565_hex(data.color)}, size_{data.size} {
		int16_t X, Y; // discard results
		frame.setTextSize(size_);
		frame.getTextBounds(text_, x_, y_, &X, &Y, &width_, &height_);
		height_ = max_text_height();
	}

	void draw() const {
		frame_.setTextColor(color_565_);
		frame_.setTextSize(size_);
		frame_.setCursor(x_ - size_, y_ + height_ - size_ * 3);
		frame_.println(text_);
	}

	[[nodiscard]] auto color() const { return color_565_; }

	static void print(GFXcanvas16 &frame, SizedColoredText data,
	                  std::uint16_t x, std::uint16_t y) {
		frame.setTextColor(to_565(data.color).hex());
		frame.setTextSize(data.size);
		frame.setCursor(x, y);
		frame.print(data.text.data());
	}

  private:
	std::uint16_t max_text_height() const {
		int16_t x, y;
		std::uint16_t width, height;
		// characters with largest height range, make text boxes more consistent
		frame_.getTextBounds("gT", x_, y_, &x, &y, &width, &height);
		return height;
	}

	const char *text_;
	std::uint16_t color_565_;
	std::uint8_t size_;
};

//===============================================================================================
// Error text display
//===============================================================================================

class ErrorDisplay : public ScreenElement {
  public:
	ErrorDisplay(GFXcanvas16 &frame, std::string_view text)
	    : ScreenElement{frame}, text_{frame, {text, 0xFFFFFF, 1}, 0, 120} {
		text_.center_x();
	}

	void draw() const {
		Text::print(frame_, {"error", 0xFF0000, 2}, 115, 80);
		text_.draw();
	}

  private:
	Text text_;
};
//===============================================================================================
// 565 color bitmap
//===============================================================================================
class Bitmap : public ScreenElement {
  public:
	Bitmap(GFXcanvas16 &frame, const std::uint16_t *bitmap565,
	       std::uint16_t width, std::uint16_t height, std::uint16_t x,
	       std::uint16_t y)
	    : ScreenElement{frame, x, y, width, height}, data_{bitmap565} {}

	void draw() const { frame_.drawRGBBitmap(x_, y_, data_, width_, height_); }

  private:
	const std::uint16_t *data_;
};

//===============================================================================================
// Startup splash screen bitmap
//===============================================================================================

struct SplashScreen : public ScreenElement {
	SplashScreen(GFXcanvas16 &frame) : ScreenElement{frame} {}

	void draw() const {
		Bitmap splash{
		    frame_,
		    assets::sandbox_logo_bitmap.data(),
		    frame_width(),
		    frame_height(),
		    0,
		    0,
		};
		splash.draw();
	}
};

//===============================================================================================
//  Labeled knob with variable turn amount
//===============================================================================================
class Knob : public ScreenElement {
  public:
	static constexpr std::uint8_t radius = 25;
	static constexpr std::uint8_t width = radius * 2 + 1;

  public:
	Knob(GFXcanvas16 &frame, std::uint16_t x, std::uint16_t y,
	     float percent_turned, std::string_view label)
	    : ScreenElement{frame, x, y, Knob::width, Knob::width},
	      percent_turned_{percent_turned}, label_{label} {}

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
		frame_.getTextBounds(label_.data(), 0, y_, &text_x, &text_y,
		                     &text_width, &text_height);

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

class PortsDisplay : public ScreenElement {
  public:
	PortsDisplay(GFXcanvas16 &frame, const ModulePortNames &labels,
	             const nst::vector_8U<nst::teensy::ColorRGB> &colors,
	             std::uint16_t x, std::uint16_t y)
	    : ScreenElement{frame, x, y, 0, square_width}, labels_{labels},
	      colors_{colors} {
		const auto num_ports = labels_.size();
		width_ = (num_ports == 0)
		             ? 0
		             : square_width * num_ports + spacing_px * (num_ports - 1);
	}

	void draw() const {
		if (labels_.size() == 0) {
			return;
		}

		for (std::size_t i{}; i < std::min(labels_.size(), colors_.size());
		     ++i) {
			int x = x_ + i * (square_width + spacing_px);
			const auto color_565 =
			    static_cast<nst::teensy::Color565>(colors_[i]);
			std::uint16_t text_color{};

			frame_.drawRoundRect(x, y_, square_width, square_width,
			                     square_radius - 1, ILI9341_DARKGREY);

			if (color_565.hex() != 0) {
				frame_.fillRoundRect(x + 1, y_ + 1, square_width - 2,
				                     square_width - 2, square_radius,
				                     color_565.hex());
			} else {
				text_color = 0x6060;
			}

			Text label(frame_, {labels_[i].view(), text_color, 1}, 0, 0);
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
	const ModulePortNames &labels_;
	const nst::vector_8U<nst::teensy::ColorRGB> &colors_;
};

class ModuleDisplay : public ScreenElement {
  public:
	ModuleDisplay(
	    GFXcanvas16 &frame, ColoredText name,
	    const ModuleControlNames &ctrl_labels,
	    const std::array<std::uint8_t, limits::max_module_ctrls> &ctrl_vals,
	    const ModulePortNames &in_names, const ModulePortNames &out_names,
	    const nst::vector_8U<nst::teensy::ColorRGB> &in_colors,
	    const nst::vector_8U<nst::teensy::ColorRGB> &out_colors)
	    : ScreenElement{frame},
	      name_{frame, {name.text, name.color, name_size}, 0, 65},
	      ctrl_labels_{ctrl_labels}, ctrl_vals_{ctrl_vals},
	      inputs_{frame, in_names, in_colors, 30, 25},
	      outputs_{frame, out_names, out_colors, 30, 110} {
		name_.center_x();
	}

	void draw() const {
		frame_.drawRoundRect(name_.x() - 2, name_.y() - 2, name_.width() + 4,
		                     name_.height() + 2, 3, name_.color());
		name_.draw();
		inputs_.draw();
		outputs_.draw();
		Text::print(frame_, {"->", 0x606060, 1}, 10, 39);
		Text::print(frame_, {"<-", 0x606060, 1}, 10, 126);
		draw_knobs();
	}

  private:
	void draw_knobs() const {
		constexpr static std::uint16_t knobs_y = 175;
		constexpr static auto spacing_px = 24;

		for (std::size_t i{}; i < 4; ++i) {
			const std::uint16_t knob_x =
			    Knob::radius + spacing_px + (Knob::radius + spacing_px * 2) * i;

			const auto turn_amt = static_cast<float>(ctrl_vals_[i]) / 255.0f;

			if (i < ctrl_labels_.size()) {
				Knob{frame_, knob_x, knobs_y, turn_amt, ctrl_labels_[i].view()}
				    .draw();
			} else {
				frame_.drawCircle(knob_x, knobs_y, Knob::radius * 0.85,
				                  ILI9341_DARKGREY);
			}
		}
	}

  public:
	static void draw_knob(float value, std::string_view label,
	                      std::size_t index, GFXcanvas16 &frame) {
		constexpr static std::uint16_t knobs_y = 175;
		constexpr static auto spacing_px = 24;

		const std::uint16_t knob_x =
		    Knob::radius + spacing_px + (Knob::radius + spacing_px * 2) * index;

		Knob{frame, knob_x, knobs_y, value, label}.draw();
	}

	static void draw_empty_knob(std::uint16_t x, std::uint16_t y,
	                            GFXcanvas16 &frame) {
		frame.drawCircle(x, y, Knob::radius * 0.85, ILI9341_DARKGREY);
	}

  private:
	constexpr static std::uint8_t name_size = 2;
	Text name_;
	const ModuleControlNames &ctrl_labels_;
	const std::array<std::uint8_t, 4> &ctrl_vals_;
	PortsDisplay inputs_;
	PortsDisplay outputs_;
};

class BankDisplayPage : public ScreenElement {
  public:
	BankDisplayPage(std::string_view name, std::string_view description,
	                std::uint32_t color, GFXcanvas16 &frame)
	    : ScreenElement{frame},
	      module_name_{
	          frame, {name, color, module_name_size}, x(), module_name_y},
	      description_{
	          frame,
	          {description, 0x606060, description_size},
	          x(),
	          static_cast<std::uint16_t>(module_name_.y() + spacing_px)} {
		module_name_.center_x();
		description_.center_x();
	}

	void draw() const {
		module_name_.draw();
		description_.draw();

		Text tip{frame_, {"Scroll with knob 1.", 0x606060, 1}, 50, 200};
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

class PatchDisplayPage : public ScreenElement {
  public:
	PatchDisplayPage(GFXcanvas16 &frame, ColoredText src_name,
	                 ColoredText dst_name, ColoredText src_port,
	                 ColoredText dst_port)
	    : ScreenElement(frame),
	      src_name_{frame, {src_name.text, src_name.color, 2}, x(), 50},
	      dst_name_{frame, {dst_name.text, src_name.color, 2}, x(), 150},
	      src_port_name_{frame, {src_port.text, src_port.color, 1}, 15, y()},
	      dst_port_name_{frame, {src_port.text, src_port.color, 1}, 15, y()} {
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
		    ScreenElement::centered_x(square_width, src_port_x, src_port_width);
		const auto src_rect_y = ScreenElement::centered_y(
		    square_width, src_port_y, src_port_height);

		const auto dst_port_x = dst_port_name_.x();
		const auto dst_port_y = dst_port_name_.y();
		const auto dst_port_width = dst_port_name_.width();
		const auto dst_port_height = dst_port_name_.height();
		const auto dst_rect_x =
		    ScreenElement::centered_x(square_width, dst_port_x, dst_port_width);
		const auto dst_rect_y = ScreenElement::centered_y(
		    square_width, dst_port_y, dst_port_height);

		frame_.drawRoundRect(src_rect_x, src_rect_y, square_width, square_width,
		                     5, src_name_.color());
		frame_.drawRoundRect(dst_rect_x, dst_rect_y, square_width, square_width,
		                     5, dst_name_.color());

		const auto arrow_x1 = src_port_x + (src_port_width / 2);
		const auto arrow_y1 = src_port_y + src_port_height + 5;
		const auto arrow_x2 = dst_port_x + (dst_port_width / 2);
		const auto arrow_y2 = dst_port_y - 5;

		draw_down_arrow(arrow_x1, arrow_y1, arrow_x2, arrow_y2,
		                src_name_.color(), frame_);
	}

  private:
	constexpr static auto square_width = 26U;
	Text src_name_;
	Text dst_name_;
	Text src_port_name_;
	Text dst_port_name_;
};

class OscilloscopeFrame : public ScreenElement {
	static constexpr std::size_t bufferSize = 1024;
	using SampleBuffer = std::array<float, bufferSize>;

  public:
	OscilloscopeFrame(const SampleBuffer &buffer, GFXcanvas16 &frame)
	    : ScreenElement(frame), buffer_(buffer) {}

	void draw() const {
		frame_.fillScreen(0);

		frame_.drawRoundRect(window_x, window_y, window_width, window_height,
		                     20, border_color);
		frame_.drawFastHLine(window_x, window_y + window_height / 2,
		                     window_width, zero_line_color);

		Text title{
		    frame_, {"scope", nst::teensy::ColorRGB{ILI9341_CYAN}, 1}, 0, 6};
		title.center_x();
		title.draw();

		uint16_t color;
		int prev_x = 0;
		int prev_y = 0;

		for (std::size_t i{}; i < buffer_.size(); i += downsample) {
			const auto sample = buffer_[i];
			const auto percent_drawn =
			    static_cast<float>(i) / (buffer_.size() - 1);

			const auto x = (window_x + 1) +
			               static_cast<int>(percent_drawn * (window_width - 1));
			const auto y = (window_height / 2) -
			               (window_height / 2) * (sample * waveform_scale) +
			               window_y;

			if (i == 0) {
				prev_x = x;
				prev_y = y;
				continue;
			}

			if (prev_x == x && prev_y == y) {
				continue;
			}

			color = (sample <= -1.0f || sample >= 1.0f) ? clipping_color
			                                            : safe_color;

			frame_.drawLine(prev_x, prev_y, x, y, color);
			prev_x = x;
			prev_y = y;
		}
	}

  private:
	constexpr static auto downsample = 8U;
	constexpr static float waveform_scale = 0.7f;
	constexpr static float window_scale = 0.85f;

	constexpr static auto window_width = frame_width() * window_scale;
	constexpr static auto window_height = frame_height() * window_scale;
	constexpr static auto window_x = centered_x(window_width, 0, frame_width());
	constexpr static auto window_y =
	    centered_y(window_height, 0, frame_height()) + 7;

	constexpr static auto zero_line_color = ILI9341_DARKGREY;
	constexpr static auto border_color = ILI9341_DARKCYAN;
	constexpr static auto safe_color = ILI9341_GREEN;
	constexpr static auto clipping_color = ILI9341_RED;

	const SampleBuffer &buffer_;
};

class KeyboardElement : public ScreenElement {
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
	    : ScreenElement{frame, 0, 60, 0, 0},
	      cur_text_{
	          frame,
	          {current_text, nst::teensy::to_rgb_hex(ILI9341_LIGHTGREY), 2},
	          0,
	          0} {
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
			std::uint16_t key_x =
			    x_ + (key_idx % 8) * (key_width + key_spacing);
			std::uint16_t key_y =
			    y_ + (key_idx / 8) * (key_width + key_spacing);

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

			Text key_label{
			    frame_, {key_labels[key_idx], ILI9341_BLACK, 1}, 0, 0};
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