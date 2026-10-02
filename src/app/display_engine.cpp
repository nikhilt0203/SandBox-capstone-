#include "display_engine.hpp"
#include "Fonts/FreeSansBoldOblique9pt7b.h"
namespace sndbx {

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

void DisplayEngine::display_module(ModulePosition pos) {
	screen_.clear();
	const auto &entry = display_grid_[pos.value];
	if (!entry) {
		return;
	}
	// const auto &ctrl_vals = entry->ctrl_vals;

	// module_page.draw();
}

void DisplayEngine::update_module_ctrl(ModulePosition pos, std::uint8_t idx,
                                       std::uint8_t value) {
	display_grid_[pos.value]->ctrl_vals[idx] = value;
}

} // namespace sndbx
