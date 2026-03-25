#ifndef SANDBOX_GRID_HPP_
#define SANDBOX_GRID_HPP_

#include <cstdint>

namespace sndbx::grid
{
  inline constexpr std::uint8_t rows = 8U;
  inline constexpr std::uint8_t cols = 8U;
  inline constexpr std::uint8_t totalCells = rows * cols;
  inline constexpr std::uint8_t bankStart = totalCells - cols; //last row is reserved for module bank

  struct Position
  {
    std::uint8_t row;
    std::uint8_t col;

    [[nodiscard]] constexpr std::uint8_t index() const noexcept { return row * cols + col; } 
    constexpr bool operator<(const Position& other) const { return index() < other.index(); }
    constexpr bool operator==(const Position& other) const { return index() == other.index(); }
  };

  [[nodiscard]] constexpr Position toPosition(std::uint8_t value) 
  { 
    return {
      static_cast<std::uint8_t>(value / cols),
      static_cast<std::uint8_t>(value % cols)
    }; 
  }

  [[nodiscard]] constexpr bool isBuildableArea(const sndbx::grid::Position& pos) { return pos.index() < bankStart; }

  [[nodiscard]] constexpr bool isBankArea(const sndbx::grid::Position& pos) { return pos.index() >= bankStart && pos.index() < totalCells; }
}

#endif