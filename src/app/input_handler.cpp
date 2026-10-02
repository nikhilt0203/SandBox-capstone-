#include "application.hpp"

#include "input_handler.hpp"
#include "modules/module_position.hpp"
#include <algorithm>
#include <optional>

namespace sndbx {

// helpers
namespace {

bool is_patch_action(const KeypadEvent &first, const KeypadEvent &second) {
	constexpr static auto timeout_ms = 1500u;
	return first.data.key_num != second.data.key_num &&
	       second.time - first.time < timeout_ms;
}

void handle_patch_action(App &app, std::size_t first_key_num,
                         std::size_t second_key_num) {
	const ModulePosition src_pos{first_key_num};
	const ModulePosition dst_pos{second_key_num};

	if (app.engine().connection_exists(src_pos, dst_pos)) {
		app.disconnect_first(src_pos, dst_pos);
	} else if (app.engine().connection_exists(dst_pos, src_pos)) {
		app.disconnect_first(dst_pos, dst_pos);
	} else {
		app.connect_first(src_pos, dst_pos);
	}
}

} // namespace

void EditMode::on_knob_evt(App &app, const KnobEvent &evt) {
	const auto [idx, delta] = evt;

	if (idx == 0 && last_key_evt_.has_value() &&
	    last_key_evt_->data.key_num > 55) {
		app.rotate_bank(delta);
	} else {
		app.turn_module_knob(idx, delta);
	}
}
static struct {
	Pressable *pressable = nullptr;
	ModulePosition pos{0};
} pressable_cache;

bool pressable_cached() { return pressable_cache.pressable; }

void reset_pressable_cache() { pressable_cache.pressable = nullptr; }

void EditMode::on_keypad_evt(App &app, const KeypadEvent &evt) {
	const auto key_num = evt.data.key_num;
	constexpr static auto hold_ms = 1000u;
	const auto last_key_num = last_key_evt_->data.key_num;

	if (last_key_evt_.has_value() && is_patch_action(*last_key_evt_, evt)) {
		handle_patch_action(app, last_key_num, key_num);
		last_key_evt_.reset();
	}

	auto &engine = app.engine();

	const auto module_pos = ModulePosition{key_num};
	auto module_id = engine.module_id(module_pos);
	Pressable *pressable = nullptr;
	if (module_id) {
		pressable = engine.factory()[*module_id].get_if<Pressable>();
	}

	switch (evt.data.edge) {
	case sndbx::KeypadEvent::Edge::RISING_EDGE:
		timer_.start();
		if (module_id) {
			app.select(module_pos);
		}
		if (pressable) {
			pressable->on_rising_edge();
			pressable_cache = {pressable, module_pos};
		}
		break;

	case sndbx::KeypadEvent::Edge::FALLING_EDGE:
		if (timer_.has_reached(hold_ms)) {
			app.delete_module(module_pos);
			if (module_pos == pressable_cache.pos) {
				reset_pressable_cache();
			}
		}
		if (pressable_cached() && module_pos == pressable_cache.pos) {
			pressable_cache.pressable->on_falling_edge();
		}
		break;
	}

	last_key_evt_ = evt;
}
void EditMode::on_button_evt(App &app, const ButtonEvent &evt) {
	Serial.printf("button %d pressed\n", evt.button_num);
}

void ViewMode::on_knob_evt(App &app, const sndbx::KnobEvent &evt) {
	app.turn_module_knob(evt.encoder_num, evt.delta);
}

void ViewMode::on_keypad_evt(App &app, const sndbx::KeypadEvent &e) {}
void ViewMode::on_button_evt(App &app, const sndbx::ButtonEvent &e) {}

} // namespace sndbx
