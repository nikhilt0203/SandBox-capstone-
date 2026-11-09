#ifndef SANDBOX_CURVES_HPP_
#define SANDBOX_CURVES_HPP_

#include <cmath>
#include <nst/ratio.hpp>

namespace sndbx::curves {

// Curve functions can accept a ratio as a template parameter. A coefficient
// type must have a static data member 'value' that contains the
// coefficient. nst::ratio<N,D> can be used to express a floating point
// value inline

template <typename T, int Coeff = 1>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	return value + delta * Coeff;
}

template <typename T, int Base>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	return delta == 0
	           ? value
	           : std::pow(static_cast<T>(Base), static_cast<T>(value + delta));
}

template <typename T, int Coeff = 1>
[[nodiscard]] inline constexpr T log(T value, int delta) {
	return delta == 0 ? value : std::log(value + delta) * Coeff;
}

template <typename T, typename Coeff>
[[nodiscard]] inline constexpr T log(T value, int delta) {
	return delta == 0 ? value : std::log(value + delta) * Coeff::value;
}

template <typename T, typename Base>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	return delta == 0 ? value
	                  : std::pow(static_cast<T>(Base::value),
	                             static_cast<T>(value + delta));
}

template <typename T, typename Coeff>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	return value + delta * Coeff::value;
}

inline constexpr auto gain_curve = exp<float, nst::ratio<108, 100>>;
inline constexpr auto frequency_curve = exp<float, nst::ratio<11, 10>>;

} // namespace sndbx::curves

#endif