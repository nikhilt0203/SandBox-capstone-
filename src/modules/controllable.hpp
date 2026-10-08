#ifndef SANDBOX_CONTROLLABLE_HPP_
#define SANDBOX_CONTROLLABLE_HPP_

#include "config/config.hpp"
#include <cstdint>

namespace sndbx {

class Controllable {
	std::uint8_t num_ctrls_;

  public:
	Controllable(std::uint8_t num_ctrls) : num_ctrls_{num_ctrls} {}
	virtual ~Controllable() = default;

	[[nodiscard]] auto num_controls() const { return num_ctrls_; }

	// Returns the new value (normalized)
	virtual float change_control(std::uint8_t idx, std::int8_t amt) = 0;
};

} // namespace sndbx

#endif