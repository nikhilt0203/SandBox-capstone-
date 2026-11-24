#include "display_engine.hpp"

#include "Fonts/FreeSansBoldOblique9pt7b.h"

namespace sndbx {

namespace {
// Helpers to convert between position index and row, col
auto to_row_col(ModulePosition pos) {
	return std::pair{pos.value / limits::grid_rows,
	                 pos.value % limits::grid_cols};
}

template <typename T = ModulePosition::underlying_t>
auto to_index(T row, T col) {
	return row * limits::grid_rows + col;
}

using PortColors =
    nst::inplace_vector<nst::teensy::ColorRGB, limits::max_in_ports>;

auto connected_module_colors(const Engine &e, ModulePosition pos) {
	PortColors input_colors;
	PortColors output_colors;

	for (auto &c : e.connections()) {
		if (c.dst_pos == pos) {
			const auto &m = e.get_module(c.src_pos);
			if (auto d = m.get_if<Displayable>()) {
				input_colors[c.input_idx] = d->led_color();
			}
		} else if (c.src_pos == pos) {
			const auto &m = e.get_module(c.dst_pos);
			if (auto d = m.get_if<Displayable>()) {
				output_colors[c.output_idx] = d->led_color();
			}
		}
	}
	return std::pair{input_colors, output_colors};
}

void init_canvas(GFXcanvas16 &c) { c.setFont(&FreeSansBoldOblique9pt7b); }

} // namespace

DisplayEngine::DisplayEngine(Adafruit_MultiTrellis &t) : led_grid_{t} {
	screen_.modify_canvas(init_canvas);
}

void DisplayEngine::render_frame() {
	screen_.render_frame();
	led_grid_.render_frame();
}

auto DisplayEngine::get_path(ModulePosition start, ModulePosition end)
    -> ConnectionPath {
	ConnectionPath path;
	const auto [end_row, end_col] = to_row_col(end);
	auto [row, col] = to_row_col(start);

	while (col != end_col) {
		if (col > end_col) {
			--col;
		}
		if (col < end_col) {
			++col;
		}
		path.push_back(to_index(row, col));
	}
	while (row != end_row) {
		if (row > end_row) {
			--row;
		}
		if (row < end_row) {
			++row;
		}
		path.push_back(to_index(row, col));
	}
	return path;
}

void DisplayEngine::draw_connection(ModulePosition src, ModulePosition dst) {
	const auto path_color =
	    (display_grid_[src.value]->module.led_color() * config::led_brightness).hex();

	auto draw_path = [this, path_color](const auto &path) {
		for (auto idx : path) {
			led_grid_.draw_pixel(idx, path_color);
		}
	};

	auto count_modules_crossed = [this](const auto &path) {
		int crossings{};
		for (auto idx : path) {
			if (display_grid_[idx]) {
				++crossings;
			}
		}
		return crossings;
	};

	// prefer the path that crosses the least number of other modules
	const auto path1 = get_path(src, dst);
	const auto path1_crossings = count_modules_crossed(path1);

	if (path1_crossings == 0) {
		draw_path(path1);
		return;
	}

	const auto path2 = get_path(dst, src);
	const auto path2_crossings = count_modules_crossed(path1);

	if (path2_crossings < path1_crossings) {
		draw_path(path2);
	}
}

void DisplayEngine::add_module(const Displayable &module, ModulePosition pos) {
	display_grid_[pos.value].emplace(module);
}

void DisplayEngine::reset(ModulePosition pos) {
	display_grid_[pos.value].reset();
}

void DisplayEngine::display_module(const Engine &engine, ModulePosition pos) {
	const auto &entry = display_grid_[pos.value];
	if (!entry) {
		return;
	}

	const auto &[ctrl_vals, module] = *entry;
	const auto &info = module.display_info();

	const auto [input_colors, output_colors] =
	    connected_module_colors(engine, pos);

	ui::ModuleDisplay module_page{module.display_text(),
	                              info.ctrl_names,
	                              info.in_names,
	                              info.out_names,
	                              ctrl_vals,
	                              input_colors,
	                              output_colors,
	                              screen_.current_frame()};
	screen_.clear();
	module_page.draw();
}

void DisplayEngine::update_module_ctrl(ModulePosition pos, std::size_t idx,
                                       float normalized_val) {
	// scale from float 0.0 - 1.0 to 0 - 255
	display_grid_[pos.value]->ctrl_vals[idx] =
	    normalized_val * std::numeric_limits<std::uint8_t>::max();
}

} // namespace sndbx
