#ifndef SANDBOX_TRELLIS_HPP_
#define SANDBOX_TRELLIS_HPP_

#include "Adafruit_NeoTrellis.h"
#include "event.hpp"
#include "pinouts.hpp"
#include <optional>
#include "core/fixed_vector.hpp"

class Trellis
{
public:
  Trellis();
  
  void update() { m_MultiTrellis.read(); }

  [[nodiscard]] std::optional<sndbx::event::TrellisPress> popEvent();
  [[nodiscard]] std::optional<sndbx::event::TrellisPress> const readEvent();

  [[nodiscard]] bool hasEvent() const { return !m_KeyEventQueue.is_empty(); }
  
  [[nodiscard]] constexpr std::size_t numCells() const noexcept { return numKeys; }
  
  [[nodiscard]] Adafruit_MultiTrellis& trellis() { return m_MultiTrellis; }
  
private:
  static void staticCallback(keyEvent evt) { s_Instance->handleEvent(evt); }
  void handleEvent(keyEvent evt);

private:
  inline static Trellis* s_Instance{};

  static constexpr std::size_t rows = 2U;
  static constexpr std::size_t cols = 2U;
  static constexpr std::size_t numKeys = rows * 4U * cols * 4U;

  inline static std::array<Adafruit_NeoTrellis, rows*cols> m_TrellisArray = {
    Adafruit_NeoTrellis{ TRELLIS_1_ADDR }, 
    Adafruit_NeoTrellis{ TRELLIS_2_ADDR },
    Adafruit_NeoTrellis{ TRELLIS_3_ADDR }, 
    Adafruit_NeoTrellis{ TRELLIS_4_ADDR }
  };

  inline static Adafruit_MultiTrellis m_MultiTrellis{m_TrellisArray.data(), rows, cols};
  
  sndbx::vector_16U<sndbx::event::TrellisPress> m_KeyEventQueue;
};

#endif