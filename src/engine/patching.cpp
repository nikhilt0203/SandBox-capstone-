#include "patching.hpp"

namespace sndbx::patch
{

bool connect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort)
{
  if (!graph.connect(static_cast<Patchable*>(src), srcPort, static_cast<Patchable*>(dest), destPort)) { 
    return false; 
  }

  src->output(srcPort).connectedModule = dest;
  dest->input(destPort).connectedModule = src;

  return true;
}

bool disconnect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort)
{
  if (!graph.disconnect(static_cast<Patchable*>(src), srcPort, static_cast<Patchable*>(dest), destPort)) { 
    return false; 
  }

  src->output(srcPort).connectedModule = nullptr;
  dest->input(destPort).connectedModule = nullptr;

  return true;
}

[[nodiscard]] std::optional<std::size_t> portIndexOfModule(Module* query, const std::vector<Module::Port>& ports)
{
  std::optional<std::size_t> port{};
  for (std::size_t i{}; i < ports.size(); i++)
  {
    if (ports[i].connectedModule == query) 
    { 
      port = i; 
      break;
    }
  }
  return port;
}

bool disconnectFirstConnection(AudioGraph& graph, Module* m1, Module* m2)
{
  if (auto destPort = portIndexOfModule(m2, m1->inputs()))
  {
    const auto srcPort = portIndexOfModule(m1, m2->outputs());
    if (!srcPort) { 
      return false; 
    } 
    return disconnect(graph, m2, *srcPort, m1, *destPort);
  }

  if (auto srcPort = portIndexOfModule(m2, m1->outputs()))
  {
    const auto destPort = portIndexOfModule(m1, m2->inputs());
    if (!destPort) { 
      return false; 
    }
    return disconnect(graph, m1, *srcPort, m2, *destPort);
  }
  
  return false;
}

void disconnectOthers(AudioGraph& graph, Module* module, const std::vector<Module::Port>& ports)
{
  for (const auto& port : ports)
  {
    const auto connectedModule = port.connectedModule;
    if (connectedModule) { 
      disconnectFirstConnection(graph, module, connectedModule); 
    }
  }
}

void disconnectAll(AudioGraph& graph, Module* module)
{
  disconnectOthers(graph, module, module->inputs());
  disconnectOthers(graph, module, module->outputs());
}

std::optional<std::size_t> firstAvailablePort(const std::vector<Module::Port>& ports)
{
  std::optional<std::size_t> openPort{};
  for (std::size_t i{}; i < ports.size(); i++)
  {
    if (ports[i].isAvailable())
    { 
      openPort = i; 
      break; 
    }
  }
  return openPort;
}

bool connectionExists(Module* m1, Module* m2)
{
  const auto& inPort = portIndexOfModule(m2, m1->inputs());
  const auto& outPort = portIndexOfModule(m2, m1->outputs());
  return (inPort.has_value() || outPort.has_value());
}

}