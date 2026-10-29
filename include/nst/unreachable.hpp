#ifndef NST_UNREACHABLE_HPP_
#define NST_UNREACHABLE_HPP_

#include <cassert>

namespace nst {
// Calls assert(false) on construction
struct unreachable {
	unreachable() { assert(false && "Unreachable path"); }
	template <typename T> operator T() { return T{}; };
};

template <typename T> struct unreachable_return {
	unreachable(T ret) { assert(false && "Unreachable path"); }
	template <typename T> operator T() { return T{}; };
};
} // namespace nst

#endif