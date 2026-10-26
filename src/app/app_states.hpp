#ifndef SANDBOX_INPUT_HANDLER_HPP_
#define SANDBOX_INPUT_HANDLER_HPP_

#include <optional>

#include "event.hpp"
#include "timer.hpp"

namespace sndbx {

class App;

struct AppEventState {
	virtual ~AppEventState() = default;
	virtual void on_knob_evt(App &, KnobEvent) = 0;
	virtual void on_keypad_evt(App &, KeypadEvent evt) = 0;
	virtual void on_button_evt(App &, ButtonEvent evt) = 0;
};

class AppEditState final : public AppEventState {
  public:
	void on_knob_evt(App &, KnobEvent) override;
	void on_keypad_evt(App &, KeypadEvent) override;
	void on_button_evt(App &, ButtonEvent) override;

  private:
	Timer timer_;
	std::optional<KeypadEvent> last_key_evt_{};
};

class AppViewState final : public AppEventState {
  public:
	void on_knob_evt(App &, KnobEvent) override;
	void on_keypad_evt(App &, KeypadEvent) override;
	void on_button_evt(App &, ButtonEvent) override;
};

} // namespace sndbx

#endif