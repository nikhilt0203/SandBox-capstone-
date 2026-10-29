#include "application.hpp"

#include <algorithm>

#include <nst/inplace_vector.hpp>
#include <nst/teensy/rotary_encoder.hpp>
#include <nst/teensy/trellis.hpp>

#include "io/pinouts.hpp"
#include "io/sd_card.hpp"

namespace sndbx {
namespace {
// Consumed by app
static InputEventQueue event_queue;

using namespace nst::teensy;

// Input devices that add events into the queue
static auto keypad = make_multitrellis<pinouts::trellis_addrs>(event_queue);
static auto knobs = make_encoder_array(pinouts::encoder_pins, event_queue);
static auto buttons = make_button_array(pinouts::button_pins, event_queue);

// App core
static Engine engine;
static DisplayEngine display_engine{keypad.multitrellis()};

//******************************************************************************
//
/**/ static App application{engine, display_engine}; // Application instance
//
//******************************************************************************

// Push any input events into the queue
void update_inputs() {
	keypad.update();

	for (auto &knob : knobs) {
		knob.update();
	}

	for (auto &button : buttons) {
		button.update();
	}
}

// Empty the queue and give events to the app
void process_events() {
	while (!event_queue.is_empty()) {
		std::visit(application, event_queue.pop());
	}
}

} // namespace

// App startup
//******************************************************************************
void app::init() {
	Serial.begin(115200);
	if (!sd_card::init()) {
		sndbx_log_error("SD card failed.");
	}
}

// Main loop
void app::loop() {
	update_inputs();
	process_events();
	application.update();
}
//******************************************************************************

namespace {
void draw_all_connections(const Engine &e, DisplayEngine &d) {
	for (const auto &c : e.connections()) {
		d.draw_connection(c.src_pos, c.dst_pos);
	}
}
// Give the display engine the initial values of a Controllable module's params
void populate_ctrl_vals(DisplayEngine &d, Controllable &c, ModulePosition pos) {
	for (std::size_t i{}; i < c.num_controls(); ++i) {
		const ControlChange dummy{i, 0};
		d.update_module_ctrl(pos, i, c.change_control(dummy));
	}
}
} // namespace

void App::update() {
	display_engine_.display_module(engine_, selected_.pos);
	display_engine_.render_frame();
}

void App::rotate_bank(std::int8_t amt) {
	const std::int8_t min =
	    -1 * std::distance(&*module_bank.begin(), bank_window_.begin());
	const std::int8_t max =
	    std::distance(bank_window_.end(), &*module_bank.end());
	bank_window_.slide(std::clamp(amt, min, max));
}

bool App::create_module(ModuleType type, ModulePosition pos) {
	auto id = engine_.create_module(type, pos);
	if (!id) {
		return false;
	}

	auto &module = engine_.get_module(*id);

	nst::invoke_if<Displayable>(module, [this, pos](const auto &m) {
		display_engine_.add_module(m, pos);
	});
	nst::invoke_if<Controllable>(module, [this, pos](auto &m) {
		populate_ctrl_vals(display_engine_, m, pos);
	});

	return true;
}

bool App::delete_module(ModulePosition pos) {
	if (engine_.delete_module(pos)) {
		display_engine_.reset(pos);
		return true;
	}
	return false;
}

bool App::connect(ModulePosition src_pos, std::uint8_t output_idx,
                  ModulePosition dst_pos, std::uint8_t input_idx) {
	if (!engine_.connect(src_pos, output_idx, dst_pos, input_idx)) {
		return false;
	}
	display_engine.draw_connection(src_pos, dst_pos);
	return true;
}

bool App::disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                     ModulePosition dst_pos, std::uint8_t input_idx) {
	if (engine_.disconnect(src_pos, output_idx, dst_pos, input_idx)) {
		return false;
	}
	draw_all_connections(engine_, display_engine_);
	return true;
}

bool App::connect_first(ModulePosition src_pos, ModulePosition dst_pos) {
	// do display stuff
	return engine_.connect_first(src_pos, dst_pos);
}

bool App::disconnect_first(ModulePosition src_pos, ModulePosition dst_pos) {
	// do display stuff
	return engine_.disconnect_first(src_pos, dst_pos);
}

void App::select(ModuleView module, ModulePosition pos) {
	selected_ = {module, pos};
	display_engine_.display_module(engine_, pos);
}

void App::turn_module_knob(std::uint8_t idx, std::int8_t amt) {
	if (!selected_.module.holds<Controllable>()) {
		return;
	}
	auto &c = selected_.module.get<Controllable>();
	if (idx < c.num_controls()) {
		return;
	}
	const ControlChange cc{idx, amt};
	const auto new_val = c.change_control(cc);
	display_engine_.update_module_ctrl(selected_.pos, idx, new_val);
	display_engine_.display_module(engine_, selected_.pos);
}

} // namespace sndbx