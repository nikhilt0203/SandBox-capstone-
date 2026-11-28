#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

#include "Arduino.h"
#include "curves.hpp"
#include <type_traits>

namespace sndbx {

namespace detail {
template <typename T> inline constexpr auto wrap(T value, T min, T max) {
	const auto lower = static_cast<long long>(min);
	const auto upper = static_cast<long long>(max);
	const long long range = upper - lower + 1;
	const long long shifted = static_cast<long long>(value) - lower;
	const long long wrapped = ((shifted % range) + range) % range;
	return static_cast<T>(wrapped + lower);
}

template <typename T> inline constexpr auto clamp(T val, T min, T max) {
	if (val < min) {
		return min;
	} else if (val > max) {
		return max;
	} else {
		return val;
	}
}
} // namespace detail

struct ClampAtBounds {};
struct WrapAtBounds {};

template <typename T, typename BoundsPolicy = ClampAtBounds>
class ModuleParameter {
	static_assert(std::is_arithmetic_v<T>);

  public:
	using value_type = T;

	constexpr ModuleParameter(T min, T max)
	    : value_{min}, min_{min}, max_{max} {}

	constexpr ModuleParameter(T min, T max, T initial) noexcept
	    : value_{initial}, min_{min}, max_{max} {}

	[[nodiscard]] constexpr auto min() const { return min_; }
	[[nodiscard]] constexpr auto max() const { return max_; }
	[[nodiscard]] constexpr auto value() const noexcept { return value_; }

	constexpr operator T() const noexcept { return value_; }

	constexpr auto &operator=(T rhs) noexcept {
		set(rhs);
		return *this;
	}

	constexpr auto &operator+=(T rhs) noexcept {
		set(value_ + rhs);
		return *this;
	}

	constexpr auto &operator-=(T rhs) noexcept {
		set(value_ - rhs);
		return *this;
	}

	constexpr auto &operator*=(T rhs) noexcept {
		set(value_ * rhs);
		return *this;
	}

	constexpr auto &operator/=(T rhs) noexcept {
		set(value_ / rhs);
		return *this;
	}

	constexpr auto &operator%=(T rhs) noexcept {
		set(value_ % rhs);
		return *this;
	}

  private:
	constexpr void set(T val) noexcept {
		if constexpr (std::is_same_v<BoundsPolicy, WrapAtBounds>) {
			value_ = detail::wrap(val, min_, max_);
		} else if constexpr (std::is_same_v<BoundsPolicy, ClampAtBounds>) {
			value_ = detail::clamp(val, min_, max_);
		}
	}

	T value_;
	T min_;
	T max_;
};

template <typename T, typename BoundsPolicy>
[[nodiscard]] constexpr auto
normalized(const ModuleParameter<T, BoundsPolicy> &param) noexcept {
	return (static_cast<float>(param.value()) -
	        static_cast<float>(param.min())) /
	       static_cast<float>(param.max() - param.min());
}

template <typename T, typename BoundsPolicy>
constexpr void denormalize(ModuleParameter<T, BoundsPolicy> &param,
                           float value) noexcept {
	param = (param.max() - param.min()) * value;
}
// Holds a curve function ptr and a member function ptr for when the parameter
// value changes
template <class Module, typename T> class ModuleControl {
	using UpdateFunc = void (Module::*)();
	using CurveFunc = T (*)(T, int);

	CurveFunc curve;
	UpdateFunc update;

  public:
	constexpr ModuleControl(CurveFunc curve, UpdateFunc update)
	    : curve{curve}, update{update} {}

	template <typename BoundsPolicy>
	float change(Module &m, ModuleParameter<T, BoundsPolicy> &param,
	             int delta) const {
		if (delta != 0) {
			const auto prev = param.value();
			param = curve(param, delta);
			if (prev != param) {
				(m.*update)(); // call the update member fn on the module if the
				               // parameter value changed
			}
		}
		return normalized(param);
	}
};

} // namespace sndbx

#endif