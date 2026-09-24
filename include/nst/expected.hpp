#ifndef NST_EXPECTED_HPP_
#define NST_EXPECTED_HPP_

#include <cassert>
#include <type_traits>
#include <variant>

namespace nst {

template <typename ErrorType> struct unexpected {
  constexpr explicit unexpected(ErrorType e) : error(e) {}
  ErrorType error;
};

template <typename ValueType, typename ErrorType> class [[nodiscard]] expected {
  std::variant<ValueType, ErrorType> storage_;
public:
  using T = ValueType;
  using E = ErrorType;
  
  constexpr expected(T val) : storage_(std::move(val)) {}
  constexpr expected(E err) : storage_(std::move(err)) {}
  constexpr expected(unexpected<E> e) : storage_(e.error) {}

  constexpr T *operator->() { return &std::get<T>(storage_); }
  constexpr T &operator*() { return std::get<T>(storage_); }

  constexpr const T *operator->() const { return &std::get<T>(storage_); }
  constexpr const T &operator*() const { return std::get<T>(storage_); }

  [[nodiscard]] constexpr T &value() { return assert_and_access<T>(); }
  [[nodiscard]] constexpr E &error() { return assert_and_access<E>(); }
  
  [[nodiscard]] constexpr const T &value() const { return assert_and_access<T>(); }
  [[nodiscard]] constexpr const E &error() const { return assert_and_access<E>(); }

  template <typename D>
  [[nodiscard]] constexpr T value_or(D&& default_val) const {
    return has_value() ? std::get<T>(storage_) : default_val;
  }

  template <typename D>
  [[nodiscard]] constexpr E error_or(D&& default_err) const {
    return !has_value() ? std::get<E>(storage_) : default_err;
  }

  [[nodiscard]] constexpr bool has_value() const noexcept {
    return std::holds_alternative<T>(storage_);
  }
  constexpr operator bool() const noexcept { return has_value(); }

private:
  template <typename Value> [[nodiscard]] auto &assert_and_access() {
    assert(std::holds_alternative<Value>(storage_));
    return std::get<Value>(storage_);
  }

  template <typename Value> [[nodiscard]] const auto &assert_and_access() const {
    assert(std::holds_alternative<Value>(storage_));
    return std::get<Value>(storage_);
  }
};

} // namespace nst

#endif