#ifndef NST_EXPECTED_HPP_
#define NST_EXPECTED_HPP_

#include <cassert>
#include <type_traits>
#include <variant>

namespace nst {

template <typename E> struct unexpected {
	constexpr explicit unexpected(E e) : error(e) {}
	E error;
};

template <typename T, typename E> class [[nodiscard]] expected {
	std::variant<T, E> data_;

  public:
	using value_type = T;
	using error_type = E;

	constexpr expected(T val) : data_{std::move(val)} {}
	constexpr expected(E err) : data_{std::move(err)} {}
	constexpr expected(unexpected<E> e) : data_(e.error) {}

	constexpr const T *operator->() const { return &std::get<T>(data_); }
	constexpr const T &operator*() const { return std::get<T>(data_); }

	[[nodiscard]] constexpr const T &value() const {
		return assert_and_access<T>();
	}
	[[nodiscard]] constexpr const E &error() const {
		return assert_and_access<E>();
	}

	constexpr T *operator->() { return &std::get<T>(data_); }
	constexpr T &operator*() { return std::get<T>(data_); }

	[[nodiscard]] constexpr T &value() { return assert_and_access<T>(); }
	[[nodiscard]] constexpr E &error() { return assert_and_access<E>(); }

	template <typename Default>
	[[nodiscard]] constexpr T value_or(Default &&val) const {
		return has_value() ? std::get<T>(data_) : val;
	}

	template <typename Default>
	[[nodiscard]] constexpr E error_or(Default &&err) const {
		return !has_value() ? std::get<E>(data_) : err;
	}

	[[nodiscard]] constexpr bool has_value() const noexcept {
		return std::holds_alternative<T>(data_);
	}

	constexpr operator bool() const noexcept { return has_value(); }

  private:
	template <typename Alt> auto &assert_and_access() {
		assert(std::holds_alternative<Alt>(data_));
		return std::get<Alt>(data_);
	}

	template <typename Alt> const auto &assert_and_access() const {
		assert(std::holds_alternative<Alt>(data_));
		return std::get<Alt>(data_);
	}
};

} // namespace nst

#endif