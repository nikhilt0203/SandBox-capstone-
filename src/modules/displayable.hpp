#ifndef SANDBOX_DISPLAYABLE_HPP_
#define SANDBOX_DISPLAYABLE_HPP_

#include <nst/color.hpp>

#include "module_display_info.hpp"
#include "ui/ui_text.hpp"

namespace sndbx {

class Displayable {
  public:
	virtual ~Displayable() = default;

	[[nodiscard]] virtual const ModuleDisplayInfo &display_info() const = 0;

	[[nodiscard]] virtual ColoredText display_text() const {
		const auto name = display_info().name;
		return ColoredText{name.text.view(), name.color};
	}

	[[nodiscard]] virtual nst::teensy::ColorRGB led_color() const {
		return display_info().name.color;
	}
};

} // namespace sndbx

#endif