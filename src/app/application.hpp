#ifndef SANDBOX_APP_HPP_
#define SANDBOX_APP_HPP_

#include <Arduino.h>

#include "app_states.hpp"
#include "display_engine.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "modules/module_bank.hpp"
#include <nst/span.hpp>

namespace sndbx::app {

void init();
void loop();

} // namespace sndbx::app

namespace sndbx {

using AppStates = std::tuple<EditMode, ViewMode>;

class App {
  public:
	App(Engine &e, DisplayEngine &d) : engine_{e}, display_engine_{d} {}
	// display_module might belong here

	void update();

	// Dispatch input events to the current app mode
	void operator()(const KeypadEvent &evt) {
		app_state_->on_keypad_evt(*this, evt);
	}

	void operator()(const KnobEvent &evt) {
		app_state_->on_knob_evt(*this, evt);
	}

	void operator()(const ButtonEvent &evt) {
		app_state_->on_button_evt(*this, evt);
	}

	bool create_module(ModuleType type, ModulePosition pos);
	bool delete_module(ModulePosition pos);

	bool connect(ModulePosition src_pos, std::uint8_t output_idx,
	             ModulePosition dst_pos, std::uint8_t input_idx);

	bool disconnect(ModulePosition src_pos, std::uint8_t output_idx,
	                ModulePosition dst_pos, std::uint8_t input_idx);

	bool connect_first(ModulePosition src_pos, ModulePosition dst_pos);
	bool disconnect_first(ModulePosition src_pos, ModulePosition dst_pos);

	void press_module(ModulePosition pos);
	void turn_module_knob(std::uint8_t idx, std::int8_t amt);

	void rotate_bank(std::int8_t amt);
	void select(ModuleView module, ModulePosition pos);

	void clear_selection() { selected_ = {}; }

	[[nodiscard]] bool has_selection() const { return selected_.module; }

	[[nodiscard]] auto selected_pos() const { return selected_.pos; }

	[[nodiscard]] auto module_type(std::size_t bank_index) const {
		return bank_window_[bank_index].type;
	}

	[[nodiscard]] const auto &engine() const { return engine_; }
	[[nodiscard]] auto &engine() { return engine_; }

	template <class State> void change_app_state() {
		app_state_ = &std::get<State>(app_states_);
	}

  private:
	AppStates app_states_;
	AppState *app_state_;

	Engine &engine_;
	DisplayEngine &display_engine_;

	inline static constexpr ModuleBank module_bank =
	    make_module_bank(ModuleBankTypes::index_sequence{});

	nst::span<const ModuleBankEntry> bank_window_{module_bank.begin(),
	                                              limits::grid_cols};

	struct {
		ModuleView module;
		ModulePosition pos;
	} selected_{};
};

} // namespace sndbx

#endif