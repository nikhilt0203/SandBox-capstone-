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

void EditMode::on_keypad_evt(App &app, const KeypadEvent &evt) {
	const ModulePosition pos{evt.data.key_num};

	auto &engine = app.engine();
	auto id = engine.module_id(pos);
	if (!id) {
		return;
	}

	auto &module = engine.get_module(*id);

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

	app.select(module, pos);
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
