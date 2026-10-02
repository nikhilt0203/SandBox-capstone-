#include "application.hpp"
#include "display_engine.hpp"
#include "event.hpp"
#include "input_handler.hpp"
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

static InputEventHandler input_handler{application};

void handle_event(const sndbx::InputEvent &e) { std::visit(input_handler, e); }

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
		handle_event(event_queue.pop());
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

void App::update() { display_engine_.render_frame(); }

void App::rotate_bank(std::int8_t amt) {
	const std::int8_t min =
	    -1 * std::distance(&*module_bank.begin(), bank_window_.begin());
	const std::int8_t max =
	    std::distance(bank_window_.end(), &*module_bank.end());
	bank_window_.slide(std::clamp(amt, min, max));
}

bool App::create_module(ModuleType type, ModulePosition pos) {
	// display
	return engine_.create_module(type, pos).has_value();
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

auto App::module_id(ModulePosition pos) const -> std::optional<ModuleID> {
	return engine_.module_id(pos);
}

void App::select(ModulePosition pos) {
	const auto id = engine_.factory()[pos];
	selection_ = {engine_.get_module(id), pos};
}

void App::turn_module_knob(std::uint8_t idx, std::int8_t amt) {
	auto &module = selection_.module;

	if (module.holds<Controllable>() &&
	    idx < module.get<Controllable>().num_ctrls()) {
		display_engine_.update_module_ctrl(selection_.pos, idx, amt);
	}
}

} // namespace sndbx