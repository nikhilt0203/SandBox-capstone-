#include "application.hpp"

#include <algorithm>

#include <nst/inplace_vector.hpp>
#include <nst/teensy/rotary_encoder.hpp>
#include <nst/teensy/trellis.hpp>

#include "io/pinouts.hpp"
#include "io/sd_card.hpp"

namespace sndbx {
namespace {

// added to by input devices, consumed by application
static InputEventQueue event_queue;

using namespace nst::teensy;

// Input devices
static auto keypad = make_multitrellis<pinouts::trellis_addrs>(event_queue);
static auto knobs = make_encoder_array(pinouts::encoder_pins, event_queue);
static auto buttons = make_button_array(pinouts::button_pins, event_queue);

// App core
static Engine engine;
static DisplayEngine display_engine{keypad.multitrellis()};

//******************************************************************************
//
/**/ static App application; // Application instance
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

//******************************************************************************
// App startup
void app::init() {
	Serial.begin(115200);
	if (!sd_card::init()) {
		sndbx_log_error("SD card failed.");
	}
	audio::allocate_blocks();
	audio::enable_audio_shield(0.5);
}

// Main loop
void app::loop() {
	update_inputs();
	process_events();
	application.update();
}
//******************************************************************************

namespace {
void draw_all_connections() {
	for (const auto &c : engine.connections()) {
		display_engine.draw_connection(c.src_pos, c.dst_pos);
	}
}
// Give the display engine the initial values of a Controllable module's params
void populate_ctrl_vals(Controllable &c, ModulePosition pos) {
	for (std::size_t i{}; i < c.num_controls(); ++i) {
		const ControlChange dummy{i, 0};
		display_engine.update_module_ctrl(pos, i, c.change_control(dummy));
	}
}
} // namespace

void App::update() {
	display_engine.display_module(engine, selected_.pos);
	display_engine.render_frame();
}

void App::rotate_module_bank(std::int8_t amt) {
	constexpr auto max = module_bank.size();
	bank_window_ = (((bank_window_ + amt) % max) + max) % max;
}

bool create_module(ModuleType type, ModulePosition pos) {
	auto result = engine.create_module(type, pos);
	if (!result) {
		return false;
	}
	nst::invoke_if<Displayable>(*result, [pos](const auto &module) {
		display_engine.add_module(module, pos);
	});
	nst::invoke_if<Controllable>(
	    *result, [pos](auto &module) { populate_ctrl_vals(module, pos); });
	return true;
}

int idx = 0;
void App::operator()(KeypadEvent evt) {
	Serial.println("keypad");
	using nst::teensy::TrellisKeyEvent;
	if (evt.data.edge == TrellisKeyEvent::Edge::RISING_EDGE) {
		const auto pos = ModulePosition{evt.data.key_num};
		create_module(module_bank[bank_window_].type, pos);
	}
}

void App::operator()(KnobEvent evt) {
	if (evt.encoder_num == 1) {
		rotate_module_bank(evt.delta);
		return;
	}
	nst::invoke_if<Controllable>(
	    engine.get_module(engine.module_id(ModulePosition{1})),
	    [evt](auto &c) { c.change_control(ControlChange{0, evt.delta}); });
}

void App::operator()(ButtonEvent evt) {
	Serial.println("button");
	engine.connect(ModulePosition{1}, 0, ModulePosition{0}, 0);
}

} // namespace sndbx