#ifndef SANDBOX_CONTROL_CHANGE_HPP_
#define SANDBOX_CONTROL_CHANGE_HPP_

#include <cstdint>

namespace sndbx {

struct ControlChange {
	std::size_t index;
	int delta;
};

} // namespace sndbx

#endif