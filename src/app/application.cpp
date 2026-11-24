#include "application.hpp"

#include <algorithm>

#include <nst/inplace_vector.hpp>
#include <nst/teensy/rotary_encoder.hpp>
#include <nst/teensy/trellis.hpp>

#include "config/pinouts.hpp"
#include "io/sd_card.hpp"
#include "timer.hpp"

namespace sndbx {
namespace {

// added to by input devices, consumed by application
static InputEventQueue event_queue;

using namespace nst::teensy;

// Input devices
static auto keypad = make_multitrellis<pinouts::trellis_addrs>(event_queue);

static auto knobs = make_encoder_array(pinouts::encoder_pins);
static auto buttons = make_button_array(pinouts::button_pins);

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
		knob.update(event_queue);
	}
	for (auto &button : buttons) {
		button.update(event_queue);
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
		Serial.println("SD card failed.");
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
// Give the display engine the initial values of a Controllable module's params
void populate_ctrl_vals(Controllable &c, ModulePosition pos) {
	for (std::size_t i{}; i < c.num_controls(); ++i) {
		const ControlChange dummy{i, 0};
		display_engine.update_module_ctrl(pos, i, c.change_control(dummy));
	}
}

} // namespace

void App::update() {
	display_engine.draw_modules();
	display_engine.display_module(engine, selection_->pos);
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

static std::optional<KeypadEvent> saved_keypad_evt{};

void App::select_module(ModulePosition pos) {
	selection_.emplace(ModuleSelection{engine.get_module(pos), pos});
}

bool is_create_action(std::uint16_t key_num_first,
                      std::uint16_t key_num_second) {
	return key_num_first >= config::bank_start_idx &&
	       key_num_second < config::bank_start_idx;
}

void App::delete_module(ModulePosition pos) {
	engine.delete_module(pos);
	display_engine.reset(pos);
}

void App::display_module_bank() {
	auto &leds = display_engine.led_grid();

	for (std::size_t i{}; i < limits::grid_cols; ++i) {
		const auto bank_index = (bank_window_ + i) % module_bank.size();
		const auto module_color = module_bank[bank_index].name.color;
		leds.draw_pixel(config::bank_start_idx + i, module_color);
	}
}

struct TimedAction {
	std::uint32_t time_started{};
	bool is_started{};
};

void start(TimedAction &action, std::uint32_t time) {
	action.time_started = time;
	action.is_started = true;
}

void reset(TimedAction &action) {
	action.is_started = false;
	action.time_started = 0;
}

struct : TimedAction {
	ModulePosition bank_pos;
} create_module_action{};

struct : TimedAction {
	ModulePosition src_pos;
} patch_modules_action{};

struct : TimedAction {
	ModulePosition pos;
	bool timeout(std::uint32_t now_ms) { return now_ms - time_started > 1500; }
} delete_module_action{};

bool is_in_module_grid(ModulePosition pos) {
	return pos.value < config::bank_start_idx;
}

bool is_in_module_bank(ModulePosition pos) {
	return pos.value >= config::bank_start_idx;
}

void App::handle_keypad_press(KeypadEvent evt) {
	const auto key_num = evt.data.key_num;
	const auto pos = ModulePosition{key_num};
	const bool module_at_pos = engine.has_module(pos);

	auto press = [](Pressable &m) { m.on_rising_edge(); };

	switch (app_state_) {
	case State::VIEW: {
		if (module_at_pos) {
			select_module(pos);
			nst::invoke_if<Pressable>(selection_->module, press);
			break;
		}
	}
	case State::EDIT: {
		if (is_in_module_bank(pos)) {
			start(create_module_action, evt.time);
			create_module_action.bank_pos = pos;
			break;
		}

		if (module_at_pos) {
			select_module(pos);
			nst::invoke_if<Pressable>(selection_->module, press);
			start(delete_module_action, evt.time);
			start(patch_modules_action, evt.time);
		}

		if (create_module_action.is_started && is_in_module_grid(pos)) {
			const auto bank_index =
			    create_module_action.bank_pos.value - config::bank_start_idx;
			const auto module_type = module_bank[bank_index].type;

			if (create_module(module_type, pos)) {
				select_module(pos);
				nst::invoke_if<Displayable>(
				    selection_->module, [pos](const auto &d) {
					    display_engine.add_module(d, pos);
					    display_engine.display_module(engine, pos);
				    });
			}
			reset(create_module_action);
			break;
		}
		if (patch_modules_action.is_started && module_at_pos) {
			if (evt.time - patch_modules_action.time_started)
				if (!engine.connect_first(ModulePosition{last_key_num}, pos)) {
					engine.disconnect_first(ModulePosition{last_key_num}, pos);
				}
			saved_keypad_evt.reset();
		}
	}
	};
}

void App::handle_keypad_release(KeypadEvent evt) {
	const auto pos = ModulePosition{evt.data.key_num};

	if (delete_module_action.is_started &&
	    !delete_module_action.timeout(evt.time)) {
		delete_module(pos);
		reset(delete_module_action);
		return;
	}

	if (engine.has_module(pos)) {
		nst::invoke_if<Pressable>(engine.get_module(pos),
		                          [](auto &m) { m.on_falling_edge(); });
	}
}

void App::operator()(KeypadEvent evt) {
	switch (evt.data.state) {
	case KeypadEvent::State::PRESSED: {
		handle_keypad_press(evt);
		break;
	}
	case KeypadEvent::State::RELEASED: {
		handle_keypad_release(evt);
		break;
	}
	};
}

void App::operator()(KnobEvent evt) {
	if (!selection_) {
		rotate_module_bank(evt.delta);
	} else {
		nst::invoke_if<Controllable>(selection_->module, [this, evt](auto &m) {
			const auto control_idx = evt.encoder_num;
			const auto new_val =
			    m.change_control(ControlChange{control_idx, evt.delta});
			display_engine.update_module_ctrl(selection_->pos, control_idx,
			                                  new_val);
		});
	}
}

void App::operator()(ButtonEvent evt) {
	if (evt.button_num == config::mode_button_num) {
	}
}

} // namespace sndbx