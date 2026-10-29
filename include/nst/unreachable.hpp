#ifndef NST_UNREACHABLE_HPP_
#define NST_UNREACHABLE_HPP_

#include <cassert>

namespace nst {
// Calls assert(false) on construction. Denotes an unreachable line
struct unreachable {
	unreachable() { assert(false && "Unreachable path"); }
	template <typename T> operator T() { return T{}; };
};
} // namespace nst

#endif