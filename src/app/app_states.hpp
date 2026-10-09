#ifndef SANDBOX_INPUT_HANDLER_HPP_
#define SANDBOX_INPUT_HANDLER_HPP_

#include <optional>

#include "event.hpp"
#include "timer.hpp"

namespace sndbx {

class App;

class AppState {
  public:
	virtual ~AppState() = default;
	virtual void on_knob_evt(App &app, const KnobEvent &evt) = 0;
	virtual void on_keypad_evt(App &app, const KeypadEvent &evt) = 0;
	virtual void on_button_evt(App &app, const ButtonEvent &evt) = 0;
};

class EditMode final : public AppState {
  public:
	void on_knob_evt(App &app, const KnobEvent &evt) override;
	void on_keypad_evt(App &app, const KeypadEvent &evt) override;
	void on_button_evt(App &app, const ButtonEvent &evt) override;

  private:
	Timer timer_;
	std::optional<sndbx::KeypadEvent> last_key_evt_{};
};

class ViewMode final : public AppState {
  public:
	void on_knob_evt(App &app, const KnobEvent &evt) override;
	void on_keypad_evt(App &app, const KeypadEvent &evt) override;
	void on_button_evt(App &app, const ButtonEvent &evt) override;
};

} // namespace sndbx

#endif