#ifndef SANDBOX_CONTROLLABLE_HPP_
#define SANDBOX_CONTROLLABLE_HPP_

#include "control_change.hpp"
#include <cstdint>

namespace sndbx {

class Controllable {
  public:
	explicit Controllable(std::size_t num_ctrls) : num_ctrls_{num_ctrls} {}
	virtual ~Controllable() = default;

	[[nodiscard]] std::size_t num_controls() const noexcept {
		return num_ctrls_;
	}

	// Returns the new value (normalized)
	virtual float change_control(ControlChange cc) = 0;

  private:
	std::size_t num_ctrls_;
};

} // namespace sndbx

#endif