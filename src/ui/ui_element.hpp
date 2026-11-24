#ifndef SANDBOX_UI_ELEMENT_HPP_
#define SANDBOX_UI_ELEMENT_HPP_

#include <cassert>
#include <cstdint>

namespace sndbx {

template <class Frame, std::size_t FrameWidth, std::size_t FrameHeight>
class UIElement {
  public:
	UIElement(int x, int y, int width, int height, Frame &frame)
	    : x_{x}, y_{y}, width_{width}, height_{height}, frame_{frame} {}

	explicit UIElement(int width, int height, Frame &frame)
	    : x_{}, y_{}, width_{width}, height_{height}, frame_{frame} {}

	[[nodiscard]] auto x() const { return x_; }
	[[nodiscard]] auto y() const { return y_; }

	[[nodiscard]] auto width() const { return width_; }
	[[nodiscard]] auto height() const { return height_; }

	void set_x(int x) { x_ = x; }
	void set_y(int y) { y_ = y; }

	void set_width(int width) { width_ = width; }
	void set_height(int height) { height_ = height; }

	void center_x() { x_ = (FrameWidth - width_) / 2; }
	void center_y() { y_ = (FrameHeight - height_) / 2; }

	void center_x(UIElement &other) {
		x_ = other.x() + (other.width() - width_) / 2;
	}
	void center_y(UIElement &other) {
		y_ = other.y() + (other.height() - height_) / 2;
	}

	void center_x(int x, int width) { x_ = x + (width - width_) / 2; }
	void center_y(int y, int height) { y_ = y + (height - height_) / 2; }

	[[nodiscard]] static constexpr auto centered_x(int width, int parent_x,
	                                               int parent_w) {
		return parent_x + (parent_w - width) / 2;
	}

	[[nodiscard]] static constexpr auto centered_y(int height, int parent_y,
	                                               int parent_h) {
		return parent_y + (parent_h - height) / 2;
	}

	static constexpr auto frame_width = FrameWidth;
	static constexpr auto frame_height = FrameHeight;

  protected:
	int x_{};
	int y_{};
	int width_{};
	int height_{};
	Frame &frame_;
};

} // namespace sndbx

#endif