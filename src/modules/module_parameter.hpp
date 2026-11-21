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
	[[nodiscard]] constexpr auto ratio() const noexcept {
		return (static_cast<float>(value_) - static_cast<float>(min_)) /
		       static_cast<float>(max_ - min_);
	}

	void denormalize(float value) { set((max_ - min_) * value); }

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

template <typename T, typename BoundsPolicy = ClampAtBounds>
inline ModuleParameter<T, BoundsPolicy> make_param(T min, T max) {
	return {min, max};
}

// Holds a curve function ptr and a member function ptr for when the parameter
// value changes
template <class Module, typename T> class ModuleControl {
	using OnParamChange = void (Module::*)();
	using ParamCurve = T (*)(T, int);
	ParamCurve curve;
	OnParamChange update;

  public:
	// Construct a ModuleControl a custom adjustment curve
	constexpr ModuleControl(ParamCurve curve, OnParamChange update)
	    : curve{curve}, update{update} {}

	template <typename BoundsPolicy>
	constexpr float change(Module &m, ModuleParameter<T, BoundsPolicy> &param,
	                       int delta) const {
		if (delta != 0) {
			const auto old = param.value();
			param = curve(param, delta);
            Serial.println(param);
			if (old != param) {
				(m.*update)();
			}
		}
		return param.ratio();
	}
};

} // namespace sndbx

#endif