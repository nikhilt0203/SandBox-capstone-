#include "application.hpp"

#include "io/pinouts.hpp"
#include <algorithm>
#include <nst/hardware/rotary_encoder.hpp>
#include <nst/hardware/trellis.hpp>
#include <nst/inplace_vector.hpp>

namespace sndbx {

namespace {

static InputEventQueue event_queue;

static auto keypad =
    nst::teensy::make_multitrellis<pinouts::trellis_addrs>(event_queue);

static auto knobs =
    nst::teensy::make_encoder_array(pinouts::encoder_pins, event_queue);

static auto buttons =
    nst::teensy::make_button_array(pinouts::button_pins, event_queue);

static DisplayEngine display_engine{keypad.multitrellis()};

//******************************************************************************
static App application{display_engine}; // Application instance
//******************************************************************************

void update_inputs() {
	keypad.update();

	for (auto &knob : knobs) {
		knob.update();
	}

	for (auto &button : buttons) {
		button.update();
	}
}

void process_events() {
	while (!event_queue.is_empty()) {
		std::visit(application, event_queue.pop());
	}
}

void populate_ctrl_vals(DisplayEngine &d, Controllable &c, ModulePosition pos) {
	// wiggle all knobs to get the initial values
	for (std::uint8_t i{}; i < c.num_controls(); ++i) {
		d.update_module_ctrl(pos, i, c.change_control(i, 1));
	}
	for (std::uint8_t i{}; i < c.num_controls(); ++i) {
		d.update_module_ctrl(pos, i, c.change_control(i, -1));
	}
}

} // namespace

void app::init() { Serial.begin(115200); }

//******************************************************************************
void app::loop() { // Main loop
	update_inputs();
	process_events();
	application.update();
}
//******************************************************************************

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

	nst::poly_view<Displayable, Controllable> module = engine_.get_module(*id);

	if (auto d = module.get_if<Displayable>()) {
		display_engine_.add_module(*d, pos);
	}
	if (auto c = module.get_if<Controllable>()) {
		populate_ctrl_vals(display_engine_, *c, pos);
	}
	return true;
}

bool App::delete_module(ModulePosition pos) {
	// display
	return engine_.delete_module(pos);
}

bool App::connect(ModulePosition src_pos, std::uint8_t output_idx,
                  ModulePosition dst_pos, std::uint8_t input_idx) {
	// do display stuff
	return engine_.connect(src_pos, output_idx, dst_pos, input_idx);
}

bool App::disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                     ModulePosition dst_pos, std::uint8_t input_idx) {
	// do display stuff
	return engine_.disconnect(src_pos, output_idx, dst_pos, input_idx);
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
	const auto new_val = c.change_control(idx, amt);
	display_engine_.update_module_ctrl(selected_.pos, idx, new_val);
	display_engine_.display_module(engine_, selected_.pos);
}

} // namespace sndbx