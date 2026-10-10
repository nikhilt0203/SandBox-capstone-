#include "application.hpp"

#include "app_states.hpp"
#include "modules/factory/module_position.hpp"
#include <algorithm>
#include <optional>

namespace sndbx {

// helpers
namespace {

bool is_patch_action(const KeypadEvent &first, const KeypadEvent &second) {
	constexpr static auto timeout_ms = 1500u;
	const auto key_num1 = first.data.key_num;
	const auto key_num2 = second.data.key_num;
	return key_num1 != key_num2 && key_num1 < config::bank_start_idx &&
	       key_num2 < config::bank_start_idx &&
	       (second.time - first.time < timeout_ms);
}

void handle_patch_action(App &app, ModulePosition first,
                         ModulePosition second) {
	if (app.engine().connection_exists(first, second)) {
		app.disconnect_first(first, second);
	} else if (app.engine().connection_exists(second, first)) {
		app.disconnect_first(second, first);
	} else {
		app.connect_first(first, second);
	}
}

} // namespace

void AppEditState::on_knob_evt(App &app, const KnobEvent &evt) {
	const auto [idx, delta] = evt;

	if (idx == 0 && last_key_evt_.has_value() &&
	    last_key_evt_->data.key_num > 55) {
		app.rotate_bank(delta);
	} else {
		app.turn_module_knob(idx, delta);
	}
}

void AppEditState::on_keypad_evt(App &app, const KeypadEvent &evt) {
	const auto key_num = evt.data.key_num;
	const ModulePosition pos{key_num};

	auto id = app.engine().module_id(pos);
	if (!id) {
        last_key_evt_.reset();
		return;
	}

	auto &module = app.engine().get_module(*id);

	if (module.holds<Pressable>()) {
		auto &p = module.get<Pressable>();

		switch (evt.data.edge) {
		case KeypadEvent::Edge::RISING_EDGE:
			p.on_rising_edge();
			break;
		case KeypadEvent::Edge::FALLING_EDGE:
			p.on_falling_edge();
			break;
		}
	}

	if (last_key_evt_) {
		const auto last_key_num = last_key_evt_->data.key_num;

		if (is_patch_action(*last_key_evt_, evt)) {
			const ModulePosition last_pos{last_key_num};
			if (app.engine().module_id(last_pos)) {
				handle_patch_action(app, last_pos, pos);
			}
			last_key_evt_.reset();
		}

		if (last_key_num >= config::bank_start_idx &&
		    key_num < config::bank_start_idx) {
			const auto bank_index = last_key_num - config::bank_start_idx;
			app.create_module(app.module_type(bank_index), pos);
			last_key_evt_.reset();
		}
	}

	if (evt.data.edge == KeypadEvent::Edge::RISING_EDGE) {
		last_key_evt_ = evt;
	}

	app.select(module, pos);
}

void AppEditState::on_button_evt(App &app, const ButtonEvent &evt) {
	Serial.printf("button %d pressed\n", evt.button_num);
}

void AppViewState::on_knob_evt(App &app, const sndbx::KnobEvent &evt) {
	app.turn_module_knob(evt.encoder_num, evt.delta);
}

void AppViewState::on_keypad_evt(App &app, const sndbx::KeypadEvent &e) {}
void AppViewState::on_button_evt(App &app, const sndbx::ButtonEvent &e) {}

} // namespace sndbx
