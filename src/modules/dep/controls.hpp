#ifndef SANDBOX_CONTROLS_HPP_
#define SANDBOX_CONTROLS_HPP_

#include <vector>
#include <functional>
#include "core/fixed_vector.hpp"

class Controls
{
private:
  sndbx::vector_4U<std::function<void(int)>> m_ControlFuncs;

public:
  template<typename... Fs>
  Controls(Fs... funcs) : m_ControlFuncs{funcs...} {}

  void change(std::size_t index, int delta) 
  { 
    if (index < m_ControlFuncs.size()) { m_ControlFuncs[index](delta); }
  }
  
  [[nodiscard]] std::size_t size() const { return m_ControlFuncs.size(); }
};

#endif