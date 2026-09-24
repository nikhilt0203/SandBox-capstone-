#ifndef SANDBOX_MODULE_HPP_
#define SANDBOX_MODULE_HPP_

#include "audio/audio_graph.hpp"
#include <cstdint>
#include <nst/inplace_vector.hpp>

class Module : public Patchable {
public:
  struct Port {
    Module *connectedModule{};
    std::size_t index;

    Port() = default;
    constexpr Port(std::size_t index) : index(index) {}

    [[nodiscard]] bool isAvailable() const { return !connectedModule; }
  };

  static constexpr std::size_t maxPorts = 8;
  using PortArray = nst::inplace_vector<Port, maxPorts>;

public:
  Module(std::size_t inputs, std::size_t outputs) {
    for (std::size_t i{}; i < inputs; ++i) {
      m_Inputs.emplace_back(i);
    }
    for (std::size_t i{}; i < outputs; ++i) {
      m_Outputs.emplace_back(i);
    }
  }

  virtual ~Module() = default;

  [[nodiscard]] std::size_t numInputs() const { return m_Inputs.size(); }
  [[nodiscard]] std::size_t numOutputs() const { return m_Outputs.size(); }

  [[nodiscard]] Port &input(std::size_t index) { return m_Inputs.at(index); }
  [[nodiscard]] Port &output(std::size_t index) { return m_Outputs.at(index); }

  [[nodiscard]] const PortArray &inputs() const { return m_Inputs; }
  [[nodiscard]] const PortArray &outputs() const { return m_Outputs; }

private:
  PortArray m_Inputs;
  PortArray m_Outputs;
};

#endif