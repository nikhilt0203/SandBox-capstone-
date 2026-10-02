#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_
#include <algorithm>
#include <limits>
#include <type_traits>

namespace sndbx {

// Wrapper for an arithmetic type, clamps operations to [min, max]
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
class ModuleParameter {
  public:
	using value_type = T;

	constexpr ModuleParameter(T min, T max) noexcept
	    : value_{}, min_{min}, max_{max} {}

	constexpr ModuleParameter(T min, T max, T initial) noexcept
	    : value_{initial}, min_{min}, max_{max} {
		assert(initial < max);
	}

	[[nodiscard]] constexpr T min() const { return min_; }
	[[nodiscard]] constexpr T max() const { return max_; }

	constexpr operator T() const noexcept { return value_; }

	constexpr ModuleParameter &operator=(T rhs) noexcept {
		set_clamped(rhs);
		return *this;
	}

	constexpr ModuleParameter &operator+=(T rhs) noexcept {
		set_clamped(value_ + rhs);
		return *this;
	}

	constexpr ModuleParameter &operator-=(T rhs) noexcept {
		set_clamped(value_ - rhs);
		return *this;
	}

	constexpr ModuleParameter &operator*=(T rhs) noexcept {
		set_clamped(value_ * rhs);
		return *this;
	}

	constexpr ModuleParameter &operator/=(T rhs) noexcept {
		set_clamped(value_ / rhs);
		return *this;
	}

	constexpr ModuleParameter &operator%=(T rhs) noexcept {
		set_clamped(value_ % rhs);
		return *this;
	}

  private:
	constexpr void set_clamped(T val) noexcept {
		value_ = std::clamp(val, min_, max_);
	}

	T value_;
	T min_;
	T max_;
};

template <typename To, typename From>
[[nodiscard]] inline constexpr To
scale_to(const ModuleParameter<From> &p) noexcept {
	return std::numeric_limits<To>::max() *
	       static_cast<float>(static_cast<From>(p) - p.min()) /
	       static_cast<float>(p.max() - p.min());
}

} // namespace sndbx

#endif