#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

#include "curves.hpp"
#include <type_traits>

namespace sndbx {

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

	[[nodiscard]] constexpr T min() const { return min_; }
	[[nodiscard]] constexpr T max() const { return max_; }
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

	[[nodiscard]] constexpr auto ratio() const noexcept {
		return (static_cast<float>(value_) - static_cast<float>(min_)) /
		       static_cast<float>(max_ - min_);
	}

  private:
	constexpr void set(T val) noexcept {
		if constexpr (std::is_same_v<BoundsPolicy, WrapAtBounds>) {
			const auto lower = static_cast<long long>(min_);
			const auto upper = static_cast<long long>(max_);
			const long long range = upper - lower + 1;
			const long long shifted = static_cast<long long>(val) - lower;
			const long long wrapped = ((shifted % range) + range) % range;
			value_ = static_cast<T>(wrapped + lower);
		} else if constexpr (std::is_same_v<BoundsPolicy, ClampAtBounds>) {
			if (val < min_) {
				value_ = min_;
			} else if (val > max_) {
				value_ = max_;
			} else {
				value_ = val;
			}
		}
	}

	T value_;
	T min_;
	T max_;
};

// Holds a curve function ptr and a member function ptr for when the parameter
// value changes
template <class Module, typename T> class ModuleControl {
	using OnParamChange = void (Module::*)();
	using ParamCurve = T (*)(T, int);
	ParamCurve curve;
	OnParamChange update;

  public:
	constexpr ModuleControl(ParamCurve curve, OnParamChange update)
	    : curve{curve}, update{update} {}

	template <typename BoundsPolicy>
	constexpr float change(Module &m, ModuleParameter<T, BoundsPolicy> &param,
	                       int delta) const {
		const auto old = param.value();
		param = curve(param, delta);
		if (old != param) {
			(m.*update)();
		}
		return param.ratio();
	}
};

} // namespace sndbx

#endif