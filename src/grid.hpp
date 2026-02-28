#ifndef SANDBOX_GRID_HPP_
#define SANDBOX_GRID_HPP_

#include <cstdint>

namespace sndbx::grid
{
  inline constexpr std::size_t rows = 8U;
  inline constexpr std::size_t cols = 8U;
  inline constexpr std::size_t totalCells = rows * cols;
  inline constexpr std::size_t bankStart = totalCells - cols;

  struct Position
  {
    std::size_t row;
    std::size_t col;

    [[nodiscard]] constexpr std::size_t index() const noexcept { return row * cols + col; } 
    constexpr bool operator<(const Position& other) const { return index() < other.index(); }
    constexpr bool operator==(const Position& other) const { return index() == other.index(); }
  };

  [[nodiscard]] constexpr Position toPosition(std::size_t value) { return Position{value / cols, value % cols}; }

  [[nodiscard]] constexpr bool isBuildableArea(const sndbx::grid::Position& pos) { return pos.index() >= 0 && pos.index() < bankStart; }

  [[nodiscard]] constexpr bool isBankArea(const sndbx::grid::Position& pos) { return pos.index() >= bankStart && pos.index() < totalCells; }
}

#endif