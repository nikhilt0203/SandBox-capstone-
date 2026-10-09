#ifndef SANDBOX_DISPLAYABLE_HPP_
#define SANDBOX_DISPLAYABLE_HPP_

#include "module_display_info.hpp"
#include "ui/ui_text.hpp"
#include <config/config.hpp>
#include <nst/color.hpp>
#include <nst/inplace_string.hpp>
#include <nst/inplace_vector.hpp>
#include <nst/strong_alias.hpp>

namespace sndbx {

class Displayable {
  public:
	virtual ~Displayable() = default;

	[[nodiscard]] virtual const ModuleDisplayInfo &display_info() const {
		return module_info<struct Default>;
	}

	[[nodiscard]] virtual ColoredText display_text() const {
		return {"unnamed", 0xFFFFFF};
	}

	[[nodiscard]] virtual nst::teensy::ColorRGB led_color() const {
		return 0xFFFFFF;
	}
};

} // namespace sndbx

#endif