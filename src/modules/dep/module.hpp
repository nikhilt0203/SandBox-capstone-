#ifndef module_hpp_
#define module_hpp_

#include "audio/audio_graph.hpp"
#include <cstdint>

class Module 
: public Patchable
{
public:
  struct Port
  {
    Module* connectedModule{};
    std::size_t index;
    [[nodiscard]] bool isAvailable() const { return !connectedModule; }
  };

public:
  Module(std::size_t inputs, std::size_t outputs);
  virtual ~Module() = default;

  [[nodiscard]] std::size_t numInputs() const { return m_Inputs.size(); }
  [[nodiscard]] std::size_t numOutputs() const { return m_Outputs.size(); }

  [[nodiscard]] Port& input(std::size_t index) { return m_Inputs.at(index); }
  [[nodiscard]] Port& output(std::size_t index) { return m_Outputs.at(index); }

  [[nodiscard]] const std::vector<Port>& inputs() const { return m_Inputs; }
  [[nodiscard]] const std::vector<Port>& outputs() const { return m_Outputs; }

  [[nodiscard]] std::uint32_t id() const { return m_ID; }
  void setID(std::uint32_t id) { m_ID = id; }

protected:
  std::vector<Port> m_Inputs;
  std::vector<Port> m_Outputs;
  std::uint32_t m_ID;
};

#endif