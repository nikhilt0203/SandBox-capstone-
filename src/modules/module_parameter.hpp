#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

#include <type_traits>

namespace sndbx {

struct WrapAtBounds {};
struct ClampAtBounds {};

template <typename T, typename BoundsPolicy = ClampAtBounds,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
class ModuleParameter {
  public:
	using value_type = T;

	constexpr ModuleParameter(T min, T max) : value_{}, min_{min}, max_{max} {}

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

	[[nodiscard]] constexpr auto range() const noexcept { return max_ - min_; }

  private:
	constexpr void set(T val) noexcept {
		if constexpr (std::is_same_v<WrapAtBounds, BoundsPolicy>) {
			value_ =
			    (((value_ + val) - min_) % range() + range()) % range() + min_;
		} else if constexpr (std::is_same_v<ClampAtBounds, BoundsPolicy>) {
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

template <class Module, class Parameter> class ModuleControl {
	using T = typename Parameter::value_type;
	using OnChange = void (Module::*)();

  public:
	constexpr ModuleControl(Module &module, Parameter &param, OnChange func)
	    : param_{param}, module_{module}, update_func_{func} {}

	constexpr ModuleControl() = delete;
	constexpr ModuleControl(ModuleControl &&) = delete;
	constexpr ModuleControl(const ModuleControl &) = delete;
	constexpr ModuleControl &operator=(const ModuleControl &) = delete;

	constexpr auto &operator=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p = v; });
	}

	constexpr auto &operator+=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p += v; });
	}

	constexpr auto &operator-=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p -= v; });
	}

	constexpr auto &operator*=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p *= v; });
	}

	constexpr auto &operator/=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p /= v; });
	}

	constexpr auto &operator%=(T rhs) noexcept {
		return update_if_changed(rhs, [](auto &p, auto v) { return p %= v; });
	}

  private:
	template <typename Op> constexpr auto &update_if_changed(T rhs, Op &&op) {
		const auto old = param_.value();
		op(param_, rhs);
		if (old != param_) {
			(module_.*update_func_)();
		}
		return *this;
	}

	[[nodiscard]] friend float ratio(const ModuleControl &c) {
		return c.param_.ratio();
	}

	Parameter &param_;
	Module &module_;
	OnChange update_func_;
};

} // namespace sndbx

#endif