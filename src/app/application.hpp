#ifndef SANDBOX_APP_HPP_
#define SANDBOX_APP_HPP_

#include <nst/span.hpp>

#include "display_engine.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "modules/factory/module_bank.hpp"

namespace sndbx::app {

void init();
void loop();

} // namespace sndbx::app

namespace sndbx {

// interpret user input events
class App {
  public:
	void update();

	void operator()(KeypadEvent evt);
	void operator()(KnobEvent evt);
	void operator()(ButtonEvent evt);

  private:
	enum class State { EDIT, VIEW } app_state_;
	std::size_t bank_window_{};
	struct ModuleSelection {
		ModuleView module;
		ModulePosition pos;
	};
	std::optional<ModuleSelection> selection_{};

	void handle_keypad_press(KeypadEvent evt);
	void handle_keypad_release(KeypadEvent evt);

	void select_module(ModulePosition pos);

	void rotate_module_bank(std::int8_t amt);

	void display_module_bank();

	void delete_module(ModulePosition pos);
};

} // namespace sndbx

#endif