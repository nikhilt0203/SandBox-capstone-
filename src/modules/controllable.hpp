#ifndef SANDBOX_CONTROLLABLE_HPP_
#define SANDBOX_CONTROLLABLE_HPP_

#include "config/config.hpp"
#include <cstdint>

namespace sndbx {

struct ControlChange {
	std::size_t index;
	int amt;
};

class Controllable {
  public:
	Controllable(std::size_t num_ctrls) : num_ctrls_{num_ctrls} {}
	virtual ~Controllable() = default;

	[[nodiscard]] std::size_t num_controls() const { return num_ctrls_; }

	// Returns the new value (normalized)
	virtual float change_control(ControlChange cc) = 0;

  private:
	std::size_t num_ctrls_;
};

} // namespace sndbx

#endif