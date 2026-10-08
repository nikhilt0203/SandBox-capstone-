#ifndef SANDBOX_UI_ELEMENT_HPP_
#define SANDBOX_UI_ELEMENT_HPP_

#include <cstdint>

namespace sndbx {

template <class Frame, std::size_t FrameWidth, std::size_t FrameHeight>
class UIElement {
  public:
	UIElement(std::uint16_t x, std::uint16_t y, std::uint16_t width,
	          std::uint16_t height, Frame &frame)
	    : x_{x}, y_{y}, width_{width}, height_{height}, frame_{frame} {}

	explicit UIElement(Frame &frame)
	    : x_{0}, y_{0}, width_{FrameWidth}, height_{FrameHeight},
	      frame_{frame} {}

	[[nodiscard]] auto x() const { return x_; }
	[[nodiscard]] auto y() const { return y_; }

	[[nodiscard]] auto width() const { return width_; }
	[[nodiscard]] auto height() const { return height_; }

	void set_x(std::uint16_t x) { x_ = x; }
	void set_y(std::uint16_t y) { y_ = y; }

	void set_width(std::uint16_t width) { width_ = width; }
	void set_height(std::uint16_t height) { height_ = height; }

	void center_x() { x_ = (FrameWidth - width_) / 2; }
	void center_y() { y_ = (FrameHeight - height_) / 2; }

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

	[[nodiscard]] static constexpr auto frame_width() { return FrameWidth; }
	[[nodiscard]] static constexpr auto frame_height() { return FrameHeight; }

  protected:
	std::uint16_t x_;
	std::uint16_t y_;
	std::uint16_t width_;
	std::uint16_t height_;
	Frame &frame_;
};

} // namespace sndbx

#endif