#ifndef grid_hpp_
#define grid_hpp_

#include <cstdint>

namespace sndbx::grid
{
  static constexpr std::size_t rows = 8U;
  static constexpr std::size_t cols = 8U;
  static constexpr std::size_t totalCells = rows * cols;
  static constexpr std::size_t bankStart = totalCells - cols;

  struct Position
  {
    std::size_t row;
    std::size_t col;

    [[nodiscard]] constexpr std::size_t index() const noexcept { return row * cols + col; } 
    bool operator<(const Position& other) const { return index() < other.index(); }
    bool operator==(const Position& other) const { return index() == other.index(); }
  };

  [[nodiscard]] inline Position toPosition(std::size_t value) { return Position{value / cols, value % cols}; }

  [[nodiscard]] inline bool isBuildableArea(const sndbx::grid::Position& pos) { return pos.index() >= 0 && pos.index() < bankStart; }

  [[nodiscard]] inline bool isBankArea(const sndbx::grid::Position& pos) { return pos.index() >= bankStart && pos.index() < totalCells; }
}

#endif