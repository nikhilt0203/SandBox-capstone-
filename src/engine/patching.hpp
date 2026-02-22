#ifndef patching_hpp_
#define patching_hpp_

#include "modules/dep/module.hpp"
#include <optional>

namespace sndbx::patch
{
  bool connect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort)
  {
    bool success = graph.connect(static_cast<Patchable*>(src), srcPort, static_cast<Patchable*>(dest), destPort);

    if (!success) { return false; }

    src->output(srcPort).m_Connected = dest;
    dest->input(destPort).m_Connected = src;

    return true;
  }

  bool disconnect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort)
  {
    bool success = graph.disconnect(static_cast<Patchable*>(src), srcPort, static_cast<Patchable*>(dest), destPort);

    if (!success) { return false; }

    src->output(srcPort).m_Connected = nullptr;
    dest->input(destPort).m_Connected = nullptr;
  
    return true;
  }

  [[nodiscard]] bool inputAvailable(Module* module, std::size_t port)
  {
    if (port >= module->numInputs()) { return false; }
    return module->input(port).m_Connected == nullptr;
  }

  [[nodiscard]] bool outputAvailable(Module* module, std::size_t port)
  {
    if (port >= module->numOutputs()) { return false; }
    return module->output(port).m_Connected == nullptr;
  }

  [[nodiscard]] std::optional<std::size_t> firstAvailablePort(const std::vector<Module::Port>& ports)
  {
    std::optional<std::size_t> openPort{};
    for (std::size_t i{}; i < ports.size(); i++)
    {
      if (ports.at(i).available()) 
      { 
        openPort = i; 
        break; 
      }
    }
    return openPort;
  }

  bool connectFirstAvailablePorts(AudioGraph& graph, Module* src, Module* dest)
  {
    std::optional<std::size_t> srcOutPort = firstAvailablePort(src->outputs());
    if (!srcOutPort) { return false; }

    std::optional<std::size_t> destInPort = firstAvailablePort(dest->inputs());
    if (!destInPort) { return false; }

    return connect(graph, src, *srcOutPort, dest, *destInPort);
  }

  [[nodiscard]] std::optional<std::size_t> portIndexOfModule(Module* query, const std::vector<Module::Port>& ports)
  {
    std::optional<std::size_t> port{};
    for (std::size_t i{}; i < ports.size(); i++)
    {
      if (ports[i].m_Connected == query) 
      { 
        port = i; 
        break; 
      }
    }
    return port;
  }

  bool disconnectFirstConnection(AudioGraph& graph, Module* m1, Module* m2)
  {
    auto destPort = portIndexOfModule(m2, m1->inputs());
    if (destPort.has_value()) 
    {
      const auto srcPort = portIndexOfModule(m1, m2->outputs());
      if (!srcPort) { return false; } //shouldn't happen
      return disconnect(graph, m2, *srcPort, m1, *destPort);
    }
  
    const auto srcPort = portIndexOfModule(m2, m1->outputs());
    if (!srcPort) { return false; }

    destPort = portIndexOfModule(m1, m2->inputs());
    if (!destPort) { return false; } //shouldn't happen
    return disconnect(graph, m1, *srcPort, m2, *destPort);
  }

  [[nodiscard]] bool connectionExists(Module* m1, Module* m2)
  {
    //no connection if m2 is not in m1's outputs or inputs
    const auto& inPort = portIndexOfModule(m2, m1->inputs());
    const auto& outPort = portIndexOfModule(m2, m1->outputs());
    return (inPort.has_value() || outPort.has_value());
  }

  [[nodiscard]] bool portsFull(const std::vector<Module::Port>& ports)
  {
    auto it = std::find_if(ports.begin(), ports.end(), [](const Module::Port& port){ return port.available(); });
    return (it == ports.end());
  }
}

#endif