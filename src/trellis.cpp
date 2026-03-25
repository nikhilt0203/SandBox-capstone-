#include "trellis.hpp"
#include "grid.hpp"

Trellis::Trellis()
{             
  s_Instance = this; 

  if (!m_MultiTrellis.begin())
  { 
    Serial.println("Error: failed to begin trellis"); 
    return;
  }

  for (std::size_t i{}; i < numKeys; ++i)
  {
    m_MultiTrellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING, true);
    m_MultiTrellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING, true);
    m_MultiTrellis.registerCallback(i, staticCallback);
    delay(10);
  }
}

std::optional<sndbx::event::TrellisPress> Trellis::popEvent() 
{ 
  if (m_KeyEventQueue.is_empty()) { return std::nullopt; }
  auto event = m_KeyEventQueue.back();
  m_KeyEventQueue.pop_back();
  return event; 
}
  
std::optional<sndbx::event::TrellisPress> const Trellis::readEvent() 
{ 
  if (m_KeyEventQueue.is_empty()) { return std::nullopt; }
  return m_KeyEventQueue.back(); 
}
  
void Trellis::handleEvent(keyEvent evt)
{
  auto keyPress = evt.bit;

  auto edge = 
    keyPress.EDGE == SEESAW_KEYPAD_EDGE_RISING 
    ? sndbx::event::Edge::RISING_EDGE
    : sndbx::event::Edge::FALLING_EDGE;

  auto position = sndbx::grid::Position{
    static_cast<std::uint8_t>(keyPress.NUM / sndbx::grid::rows),
    static_cast<std::uint8_t>(keyPress.NUM % sndbx::grid::cols)
  };

  m_KeyEventQueue.emplace_back(position, edge);
}