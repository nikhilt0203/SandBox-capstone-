#include "display_engine.hpp"
#include "Fonts/FreeSansBoldOblique9pt7b.h"

namespace sndbx {

using PortColors =
    nst::inplace_vector<nst::teensy::ColorRGB, limits::max_in_ports>;

auto connected_module_colors(Engine &e, ModulePosition pos)
    -> std::pair<PortColors, PortColors> {
	const auto &factory = e.factory();
	PortColors input_colors;
	PortColors output_colors;

	for (const auto &c : e.connections()) {
		if (c.dst_pos == pos) {
			const auto id = factory[c.src_pos];
			const auto m = factory[id];
			if (auto d = m.get_if<Displayable>()) {
				input_colors[c.input_idx] = d->led_color();
			}
		} else if (c.src_pos == pos) {
			const auto id = factory[c.dst_pos];
			const auto m = factory[id];
			if (auto d = m.get_if<Displayable>()) {
				output_colors[c.output_idx] = d->led_color();
			}
		}
	}
	return {input_colors, output_colors};
}

DisplayEngine::DisplayEngine(Adafruit_MultiTrellis &t) : led_grid_{t} {
	screen_.for_each_buffer(
	    [](auto b) { b->setFont(&FreeSansBoldOblique9pt7b); });
}

void DisplayEngine::render_frame() {
	screen_.render_frame();
	led_grid_.render_frame();
}

void DisplayEngine::add_module(Displayable &module, ModulePosition pos) {
	display_grid_[pos.value].emplace(module);
}

void DisplayEngine::display_module(Engine &e, ModulePosition pos) {
	screen_.clear();
	const auto &entry = display_grid_[pos.value];
	if (!entry) {
		return;
	}

	const auto &d = entry->module;
	const auto &info = d.display_info();
	const auto &factory = e.factory();

	const auto [input_colors, output_colors] = connected_module_colors(e, pos);

	ModuleDisplay module_page{screen_.current_frame(),
	                          d.display_text(),
	                          info.ctrl_names,
	                          entry->ctrl_vals,
	                          info.in_names,
	                          info.out_names,
	                          input_colors,
	                          output_colors};
	module_page.draw();
}

void DisplayEngine::update_module_ctrl(ModulePosition pos, std::uint8_t idx,
                                       std::uint8_t value) {
	display_grid_[pos.value]->ctrl_vals[idx] = value;
}

} // namespace sndbx
