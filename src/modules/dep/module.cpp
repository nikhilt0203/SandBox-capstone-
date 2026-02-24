#include "module.hpp"

void setPortIndices(std::vector<Module::Port>& ports)
{
  std::size_t index{};
  for (auto& port : ports) { port.index = index++; }
}

Module::Module(std::size_t inputs, std::size_t outputs)
: m_Inputs(inputs), 
  m_Outputs(outputs) 
{
  setPortIndices(m_Inputs);
  setPortIndices(m_Outputs);
}