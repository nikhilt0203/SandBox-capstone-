#ifndef SANDBOX_DISPLAYABLE_HPP_
#define SANDBOX_DISPLAYABLE_HPP_

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
	[[nodiscard]] virtual ColoredText display_text() const = 0;
	[[nodiscard]] virtual nst::teensy::ColorRGB led_color() const = 0;
};

} // namespace sndbx

#endif