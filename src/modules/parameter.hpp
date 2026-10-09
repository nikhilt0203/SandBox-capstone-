#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

namespace sndbx {

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
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
		set_clamped(rhs);
		return *this;
	}

	constexpr auto &operator+=(T rhs) noexcept {
		set_clamped(value_ + rhs);
		return *this;
	}

	constexpr auto &operator-=(T rhs) noexcept {
		set_clamped(value_ - rhs);
		return *this;
	}

	constexpr auto &operator*=(T rhs) noexcept {
		set_clamped(value_ * rhs);
		return *this;
	}

	constexpr auto &operator/=(T rhs) noexcept {
		set_clamped(value_ / rhs);
		return *this;
	}

	constexpr auto &operator%=(T rhs) noexcept {
		set_clamped(value_ % rhs);
		return *this;
	}

	[[nodiscard]] constexpr auto ratio() const noexcept {
		return (static_cast<float>(value_) - static_cast<float>(min_)) /
		       static_cast<float>(max_ - min_);
	}

  private:
	constexpr void set_clamped(T val) noexcept {
		if (val < min_) {
			value_ = min_;
		} else if (val > max_) {
			value_ = max_;
		} else {
			value_ = val;
		}
	}

	T value_;
	T min_;
	T max_;
};

} // namespace sndbx

#endif