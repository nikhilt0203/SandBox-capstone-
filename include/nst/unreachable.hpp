#ifndef NST_UNREACHABLE_HPP_
#define NST_UNREACHABLE_HPP_

#include <cassert>

namespace nst {

namespace detail {

struct ub_on_reach {};
struct assert_on_reach {};

[[noreturn]] inline void unreachable(assert_on_reach) {
	assert(false && "Unreachable path reached.");
}

[[noreturn]] inline void unreachable(ub_on_reach) { __builtin_unreachable(); }

} // namespace detail

// Configure policy in .cpp
[[noreturn]] void unreachable();

} // namespace nst

#endif