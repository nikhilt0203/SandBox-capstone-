#ifndef NST_MATH_HPP_
#define NST_MATH_HPP_

#include <cstdint>
#include <limits>
#include <optional>

namespace nst {

template <std::size_t Value,
          typename = std::enable_if_t<std::is_integral_v<decltype(Value)> &&
                                      std::is_unsigned_v<decltype(Value)>>>
using smallest_uint = std::conditional_t<
    Value <= std::numeric_limits<std::uint8_t>::max(), std::uint8_t,
    std::conditional_t<
        Value <= std::numeric_limits<std::uint16_t>::max(), std::uint16_t,
        std::conditional_t<Value <= std::numeric_limits<std::uint32_t>::max(),
                           std::uint32_t, std::uint64_t>>>;

template <std::size_t Rows, std::size_t Cols, typename ValueType = smallest_uint<Rows * Cols>>
struct grid_coordinate {
  using value_type = ValueType;

  static constexpr ValueType rows{Rows};
  static constexpr ValueType cols{Cols};

  ValueType row;
  ValueType col;

  explicit constexpr grid_coordinate(ValueType row, ValueType col)
      : row(row), col(col) {}

  explicit constexpr grid_coordinate(ValueType index)
      : row(index / cols), col(index - (cols * row)) {}

  [[nodiscard]] constexpr ValueType index() const { return cols * row + col; }

  [[nodiscard]] constexpr bool is_in_bounds() const { return index() < rows * cols; }

  constexpr bool operator==(const grid_coordinate &other) const {
    return index() == other.index();
  }

  constexpr bool operator!=(const grid_coordinate &other) const {
    return index() != other.index();
  }

  constexpr bool operator>=(const grid_coordinate &other) const {
    return index() >= other.index();
  }

  constexpr bool operator<=(const grid_coordinate &other) const {
    return index() <= other.index();
  }

  constexpr bool operator>(const grid_coordinate &other) const {
    return index() > other.index();
  }

  constexpr bool operator<(const grid_coordinate &other) const {
    return index() < other.index();
  }

  constexpr explicit operator bool() const { return is_in_bounds(); }

  [[nodiscard]] static constexpr ValueType max() { return Rows * Cols; }
};

using Position = grid_coordinate<8, 8>;

[[nodiscard]] constexpr std::optional<Position> make(std::uint8_t index) {
  return index > Position::max() ? std::make_optional(Position{index}) : std::nullopt;
}

} // namespace nst

#endif