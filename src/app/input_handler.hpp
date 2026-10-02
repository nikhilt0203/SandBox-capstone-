#ifndef SANDBOX_INPUT_HANDLER_HPP_
#define SANDBOX_INPUT_HANDLER_HPP_

#include <optional>

#include "event.hpp"
#include "timer.hpp"

namespace sndbx {

class App;
class InputEventHandler;
class AppState {
  public:
	AppState(InputEventHandler &h) : input_handler_{h} {}
	virtual ~AppState() = default;
	virtual void on_knob_evt(App &, const KnobEvent &) = 0;
	virtual void on_keypad_evt(App &, const KeypadEvent &) = 0;
	virtual void on_button_evt(App &, const ButtonEvent &) = 0;

  protected:
	InputEventHandler &input_handler_;
};

class EditMode final : public AppState {
  public:
	using AppState::AppState;
	void on_knob_evt(App &app, const KnobEvent &evt) override;
	void on_keypad_evt(App &app, const KeypadEvent &evt) override;
	void on_button_evt(App &app, const ButtonEvent &evt) override;

  private:
	Timer timer_;
	std::optional<sndbx::KeypadEvent> last_key_evt_{};
};

class ViewMode final : public AppState {
  public:
	using AppState::AppState;
	void on_knob_evt(App &app, const KnobEvent &evt) override;
	void on_keypad_evt(App &app, const KeypadEvent &evt) override;
	void on_button_evt(App &app, const ButtonEvent &evt) override;
};

class InputEventHandler {
  public:
	using AppStates = std::tuple<EditMode, ViewMode>;

	InputEventHandler(App &app) : app_{app} {}

	void operator()(const KeypadEvent &evt) {
		state_->on_keypad_evt(app_, evt);
	}

	void operator()(const KnobEvent &evt) { state_->on_knob_evt(app_, evt); }

	void operator()(const ButtonEvent &evt) {
		state_->on_button_evt(app_, evt);
	}

	template <typename State> void change_state() {
		state_ = &std::get<State>(states_);
	}

  private:
	App &app_;
	AppStates states_{*this, *this};
	AppState *state_{&std::get<0>(states_)};
};

} // namespace sndbx

#endif