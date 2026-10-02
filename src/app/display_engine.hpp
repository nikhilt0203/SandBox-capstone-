#ifndef SANDBOX_DISPLAY_ENGINE_HPP_
#define SANDBOX_DISPLAY_ENGINE_HPP_

#include "io/pinouts.hpp"
#include "modules/displayable.hpp"
#include "modules/module_position.hpp"
#include "ui/screen_elements.hpp"
#include <nst/hardware/ILI9341_display.hpp>
#include <nst/hardware/trellis_led_display.hpp>
#include <optional>
#include <string_view>

namespace sndbx {
using Screen = nst::teensy::TFT;
using LEDGrid = nst::teensy::TrellisLEDDisplay<Adafruit_MultiTrellis>;
} // namespace sndbx

namespace sndbx::display {

template <typename Display, typename Element, typename... Args>
void draw(Display &d, Args &&...args) {
	Element{std::forward<Args>(args)..., d.current_frame()}.draw();
}

template <typename Display, typename Element, typename... Args>
void clear_and_draw(Display &d, Args &&...args) {
	d.clear();
	draw<Element>(d, std::forward<Args>(args)...);
}

template <typename Display> void clear(Display &d) { d.clear(); }

inline void print(Screen &s, std::string_view text,
                  nst::teensy::ColorRGB color = 0xFFFFFF, std::uint8_t size = 4,
                  std::uint16_t x = 0, std::uint16_t y = 0) {
	auto &frame = s.current_frame();
	frame.setTextColor(static_cast<nst::teensy::Color565>(color).hex());
	frame.setTextSize(size);
	frame.setCursor(x, y);
	frame.print(text.data());
}

} // namespace sndbx::display

namespace sndbx {

class DisplayEngine {
  public:
	explicit DisplayEngine(Adafruit_MultiTrellis &t);

	void render_frame();

	void display_module(ModulePosition pos);

	void add_module(Displayable &module, ModulePosition pos);

	void update_module_ctrl(ModulePosition pos, std::uint8_t idx,
	                        std::uint8_t value);

  private:
	struct DisplayEntry {
		DisplayEntry(Displayable &d)
		    : ctrl_vals{}, color{d.led_color()}, module{d} {}
		std::array<std::uint8_t, limits::max_module_ctrls> ctrl_vals;
		nst::teensy::ColorRGB color;
		Displayable &module;
	};

	std::array<std::optional<DisplayEntry>, config::grid_size> display_grid_{};
	Screen screen_{pinouts::tft_cs, pinouts::tft_dc};
	LEDGrid led_grid_;
};

} // namespace sndbx
#endif