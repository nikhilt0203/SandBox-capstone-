#ifndef SANDBOX_CURVES_HPP_
#define SANDBOX_CURVES_HPP_

#include <cmath>
#include <type_traits>

namespace sndbx::curves {

namespace detail {
struct IdentityCoeff {
	constexpr static auto value{1};
};
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

// template <typename T, typename Coeff = detail::IdentityCoeff,
//           typename = std::enable_if_t<!std::is_same_v<Coeff, int>>>
// [[nodiscard]] inline constexpr T log(T value, int delta) {
// 	static_assert(std::is_arithmetic_v<decltype(Coeff::value)>);
// 	if (delta == 0) {
// 		return value;
// 	}
// 	return std::log(value + delta) * Coeff::value;
// }

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

} // namespace sndbx::curves

#endif