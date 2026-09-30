#ifndef SANDBOX_UI_HPP_
#define SANDBOX_UI_HPP_

#include "nst/hardware/ILI9341_display.hpp"
#include "nst/hardware/trellis_led_display.hpp"
#include "ui/led_elements.hpp"
#include "ui/screen_elements.hpp"

namespace sndbx {

using LEDGrid = nst::teensy::TrellisLEDDisplay<Adafruit_MultiTrellis>;
using Screen = nst::teensy::TFT;

} // namespace sndbx

namespace sndbx::ui {

template <typename Display, typename Element, typename... Args>
inline void draw(Display &d, Args &&...args) {
  Element{std::forward<Args>(args)..., d.current_frame()}.draw();
}

template <typename Display, typename Element, typename... Args>
inline void draw(Display &d, const Element &e) {
  e.draw();
}

template <typename Display, typename Element, typename... Args>
inline void clear_and_draw(Display &d, Args &&...args) {
  d.clear();
  draw<Element>(d, std::forward<Args>(args)...);
}

template <typename Display> inline void clear(Display &d) { d.clear(); }

/*
 * Screen
 */
inline void print(Screen &s, std::string_view text, std::uint16_t x,
                  std::uint16_t y, std::uint32_t color, std::uint8_t pt) {
  auto &frame = s.current_frame();
  frame.setTextColor(sndbx::color::to_565(color));
  frame.setTextSize(pt);
  frame.setCursor(x, y);
  frame.print(text.data());
}

} // namespace sndbx::ui

#endif