#ifndef NST_UNREACHABLE_HPP_
#define NST_UNREACHABLE_HPP_

namespace nst {
[[noreturn]] inline void unreachable() { __builtin_unreachable(); }
} // namespace nst

#endif