#ifndef SANDBOX_CURVES_HPP_
#define SANDBOX_CURVES_HPP_

#include <cmath>
#include <type_traits>

namespace sndbx::curves {

// Curve functions can accept a ratio as a template parameter. A coefficient
// type must have a static data member 'value' that contains the coefficient.
// The type Ratio<N,D> can be used to express these values

// Compile-time ratio constant to use with curves
template <int N, int D> struct Ratio {
	static_assert(D != 0);

	constexpr operator float() const { return value; }

	static constexpr auto numerator = N;
	static constexpr auto denominator = D;
	static constexpr auto value = static_cast<float>(N) / static_cast<float>(D);
};

namespace detail {
using IdentityCoeff = Ratio<1, 1>;
} // namespace detail

template <typename T, class Coeff = detail::IdentityCoeff,
          typename = std::enable_if_t<!std::is_same_v<Coeff, int>>>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	static_assert(std::is_arithmetic_v<decltype(Coeff::value)>);
	if (delta == 0) {
		return value;
	}
	return value + delta * Coeff::value;
}

template <typename T, int Coeff>
[[nodiscard]] inline constexpr T linear(T value, int delta) {
	if (delta == 0) {
		return value;
	}
	return value + delta * Coeff;
}

template <typename T, auto Coeff = 1>
[[nodiscard]] inline constexpr T log(T value, int delta) {
	if (delta == 0) {
		return value;
	}
	return std::log(value + delta) * Coeff;
}

template <typename T, typename Base = detail::IdentityCoeff,
          typename = std::enable_if_t<!std::is_same_v<Base, int>>>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	static_assert(std::is_arithmetic_v<decltype(Base::value)>);
	if (delta == 0) {
		return value;
	}
	return std::pow(static_cast<T>(Base::value), static_cast<T>(value + delta));
}

template <typename T, int Base>
[[nodiscard]] inline constexpr T exp(T value, int delta) {
	if (delta == 0) {
		return value;
	}
	return std::pow(static_cast<T>(Base), static_cast<T>(value + delta));
}

template <typename T, typename Coeff = detail::IdentityCoeff,
          typename = std::enable_if_t<!std::is_same_v<Coeff, int>>>
[[nodiscard]] inline constexpr T sqrt(T value, int delta) {
	if (delta == 0) {
		return value;
	}
	return std::sqrt(value + delta) * Coeff::value;
}

template <typename T, int Coeff>
[[nodiscard]] inline constexpr T sqrt(T value, int delta) {
	if (delta == 0) {
		return value;
	}
	return std::sqrt(value + static_cast<T>(delta)) * Coeff;
}

inline constexpr auto default_gain_curve = exp<float, Ratio<108, 100>>;
inline constexpr auto default_freq_curve = exp<float, Ratio<11, 10>>;

} // namespace sndbx::curves

#endif