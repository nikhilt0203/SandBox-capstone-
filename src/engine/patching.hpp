#ifndef patching_hpp_
#define patching_hpp_

#include "modules/dep/module.hpp"
#include <optional>

namespace sndbx::patch
{
  bool connect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort);
  bool disconnect(AudioGraph& graph, Module* src, std::size_t srcPort, Module* dest, std::size_t destPort);
  bool disconnectFirstConnection(AudioGraph& graph, Module* m1, Module* m2);
  
  void detachFromGraph(AudioGraph& graph, Module* module);

  [[nodiscard]] std::optional<std::size_t> firstAvailablePort(const std::vector<Module::Port>& ports);
  [[nodiscard]] bool connectionExists(Module* m1, Module* m2);
}

#endif