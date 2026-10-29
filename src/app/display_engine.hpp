#ifndef SANDBOX_DISPLAY_ENGINE_HPP_
#define SANDBOX_DISPLAY_ENGINE_HPP_

#include <array>
#include <optional>

#include <nst/teensy/ILI9341_display.hpp>
#include <nst/teensy/trellis_led_display.hpp>
#include <nst/poly_view.hpp>

#include "engine.hpp"
#include "io/pinouts.hpp"
#include "modules/displayable.hpp"
#include "modules/factory/module_position.hpp"
#include "ui/screen_elements.hpp"

namespace sndbx {
using Screen = nst::teensy::TFT;
using LEDGrid =
    nst::teensy::TrellisLEDDisplay<Adafruit_MultiTrellis, limits::grid_rows,
                                   limits::grid_cols>;
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

inline void print(Screen &s, ColoredText text, std::uint32_t size = 4,
                  std::uint32_t x = 0, std::uint32_t y = 0) {
	auto &frame = s.current_frame();
	frame.setTextColor(to_565_hex(text.color));
	frame.setTextSize(size);
	frame.setCursor(x, y);
	frame.print(text.text.data());
}

} // namespace sndbx::display

namespace sndbx {

class DisplayEngine {
  public:
	explicit DisplayEngine(Adafruit_MultiTrellis &t);

	void render_frame();

	void draw_connection(ModulePosition src, ModulePosition dst);

	void display_module(const Engine &e, ModulePosition pos);

	void add_module(const Displayable &module, ModulePosition pos);

	void reset(ModulePosition pos);

	void update_module_ctrl(ModulePosition pos, std::size_t idx, float value);

  private:
	auto get_path(ModulePosition start, ModulePosition end)
	    -> nst::inplace_vector<std::uint8_t,
	                           limits::grid_rows + limits::grid_cols>;

	struct DisplayEntry {
		DisplayEntry(const Displayable &m) : color{m.led_color()}, module{m} {}
		std::array<std::uint8_t, limits::max_module_ctrls> ctrl_vals{};
		nst::teensy::ColorRGB color;
		const Displayable &module;
	};

	std::array<std::optional<DisplayEntry>, limits::grid_size> display_grid_{};
	Screen screen_{pinouts::tft_cs, pinouts::tft_dc};
	LEDGrid led_grid_;
};

} // namespace sndbx
#endif