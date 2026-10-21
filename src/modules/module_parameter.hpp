#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

#include <type_traits>

namespace sndbx {

struct ClampAtBounds {};
struct WrapAtBounds {};

template <typename T, typename BoundsPolicy = ClampAtBounds,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
class ModuleParameter {
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

template <class Module, typename Parameter> class ModuleControl {
	using T = typename Parameter::value_type;
	using OnUpdate = void (Module::*)();
	using Op = T (*)(T, int);

  public:
	constexpr ModuleControl(Op op, OnUpdate func)
	    : op_{op}, update_func_{func} {}

	float change(Module &module, Parameter &param, int x) const {
		const auto old = param.value();
		param = op_(param, x);
		if (old != param) {
			(module.*update_func_)();
		}
		return param.ratio();
	}

  private:
	Op op_;
	OnUpdate update_func_;
};

struct DefaultCoeff {
	inline static constexpr int value{1};
};

template <typename T, typename Coeff = DefaultCoeff>
[[nodiscard]] inline constexpr T linear_curve(T v, int x) {
	return v + x * Coeff::value;
}

} // namespace sndbx

#endif