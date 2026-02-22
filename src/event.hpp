#ifndef event_hpp_
#define event_hpp_

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
    std::size_t index;
    Edge edge;
    
    ButtonPress(std::size_t index, Edge e)
    : index(index), 
      edge(e) 
    {}
  };

  struct TrellisPress : public Event
  {
    sndbx::grid::Position position;
    Edge edge;
    
    TrellisPress(sndbx::grid::Position pos, Edge e)
    : position(pos), 
      edge(e) 
    {}
  };
}

#endif