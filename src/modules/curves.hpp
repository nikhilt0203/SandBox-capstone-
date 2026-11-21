#ifndef SANDBOX_CURVES_HPP_
#define SANDBOX_CURVES_HPP_

#include <cmath>
#include <nst/ratio.hpp>

namespace sndbx::curves {

// Curve functions can accept a ratio as a template parameter. A coefficient
// type must have a static data member 'value' that contains the
// coefficient. nst::ratio<N,D> or nst::percent<N> can be used to express a
// floating point value inline

template <typename T, int Coeff = 1>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	return value + delta * Coeff;
}

template <typename T, int Coeff = 1>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	return value * std::pow(static_cast<T>(Coeff), static_cast<T>(delta));
}

template <typename T, typename Coeff>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	return value + delta * Coeff::value;
}

template <typename T, typename Coeff>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	return value *
	       std::pow(static_cast<T>(Coeff::value), static_cast<T>(delta));
}

inline constexpr auto gain_curve = exp<float, nst::percent<108>>;
inline constexpr auto frequency_curve = exp<float, nst::percent<108>>;

} // namespace sndbx::curves

#endif
