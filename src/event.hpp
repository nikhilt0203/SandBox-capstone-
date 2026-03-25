#ifndef SANDBOX_EVENT_HPP_
#define SANDBOX_EVENT_HPP_

#include "grid.hpp"
#include <optional>
#include <vector>

namespace sndbx::event
{
  struct Event
  {
    unsigned long time{millis()};
  };

  enum class Edge
  {
    RISING_EDGE,
    FALLING_EDGE
  };

  struct ButtonPress : public Event
  {
    std::uint8_t index;
    Edge edge;
    
    ButtonPress(std::uint8_t index, Edge e)
    : index(index), edge(e) {}
  };

  struct EncoderTurn : public Event
  {
    std::uint8_t encoderNum;
    int delta;
    
    EncoderTurn(std::uint8_t encoderNum, int delta)
    : encoderNum(encoderNum), delta(delta) {}
  };

  struct TrellisPress : public Event
  {
    sndbx::grid::Position position;
    Edge edge;
    
    TrellisPress() = default;
    
    TrellisPress(sndbx::grid::Position pos, Edge e)
    : position(pos), edge(e) {}
  };
}

#endif