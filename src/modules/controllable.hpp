#ifndef SANDBOX_CONTROLLABLE_HPP_
#define SANDBOX_CONTROLLABLE_HPP_

#include <cstdint>

namespace sndbx {

class Controllable {
	std::uint8_t num_ctrls_;

  public:
	Controllable(std::uint8_t num_ctrls) : num_ctrls_{num_ctrls} {}
	[[nodiscard]] auto num_ctrls() const { return num_ctrls_; }

	virtual ~Controllable() = default;

	// Change control by amt and return the new value
	virtual std::uint8_t change_control(std::uint8_t idx, std::int8_t amt) = 0;
};

} // namespace sndbx

#endif