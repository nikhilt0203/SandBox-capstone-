#ifndef SANDBOX_TRELLIS_HPP_
#define SANDBOX_TRELLIS_HPP_

#include "Adafruit_NeoTrellis.h"
#include "grid.hpp"
#include "event.hpp"
#include "pinouts.hpp"
#include <optional>
#include <queue>

class Trellis
{
public:
  Trellis();
  
  void update() { m_MultiTrellis.read(); }

  [[nodiscard]] bool hasEvent() { return !m_KeyEventQueue.empty(); }
  [[nodiscard]] std::optional<sndbx::event::TrellisPress> popEvent();
  [[nodiscard]] std::optional<sndbx::event::TrellisPress> const readEvent();
  
  [[nodiscard]] constexpr std::size_t numCells() noexcept { return numKeys; }
  [[nodiscard]] Adafruit_MultiTrellis& trellis() { return m_MultiTrellis; }
  
private:
  static void staticCallback(keyEvent evt) { s_Instance->handleEvent(evt); }
  void handleEvent(keyEvent evt);

private:
  inline static Trellis* s_Instance{};

  static constexpr std::size_t rows = 2U;
  static constexpr std::size_t cols = 2U;
  static constexpr std::size_t numKeys = rows * 4U * cols * 4U;

  std::array<Adafruit_NeoTrellis, rows*cols> m_TrellisArray = {
    Adafruit_NeoTrellis{ TRELLIS_1_ADDR }, 
    Adafruit_NeoTrellis{ TRELLIS_2_ADDR },
    Adafruit_NeoTrellis{ TRELLIS_3_ADDR }, 
    Adafruit_NeoTrellis{ TRELLIS_4_ADDR }
  };

  Adafruit_MultiTrellis m_MultiTrellis{m_TrellisArray.data(), rows, cols};
  
  std::queue<sndbx::event::TrellisPress> m_KeyEventQueue;
};

#endif