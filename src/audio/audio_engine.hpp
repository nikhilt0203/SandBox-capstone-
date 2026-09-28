#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <utility>

#include "audio/audio_trigger.hpp"
#include "audio_graph.hpp"
#include "config.hpp"
#include "modules/parameter.hpp"

namespace sndbx {

// Configure audio graph
using AudioGraph =
    nst::teensy::AudioGraph<limits::max_modules, limits::max_connections>;
using AudioError = AudioGraph::Error;

template <std::size_t N>
using AudioNodeIDs = nst::inplace_vector<AudioGraph::NodeID, N>;

struct AudioEndpoint {
  AudioGraph::NodeID id;
  AudioGraph::Port port;
};

struct ModulePort {
  enum class Type : std::uint8_t { IN, OUT } type;
  std::uint8_t index;

  constexpr bool operator==(const ModulePort &other) const {
    return this->type == other.type && this->index == other.index;
  }

  constexpr bool operator!=(const ModulePort &other) const {
    return this->type != other.type || this->index != other.index;
  }
};

} // namespace sndbx

namespace sndbx::audio {

[[nodiscard]] inline auto make_endpoint(AudioGraph::NodeID id,
                                        std::uint8_t graph_port) {
  return AudioEndpoint{id, AudioGraph::Port{graph_port}};
}

// Interface between module and audio graph.
class Patchable {
public:
  Patchable(std::uint8_t ins, std::uint8_t outs) : ins_{ins}, outs_{outs} {}
  virtual ~Patchable() = default;

  [[nodiscard]] virtual auto link(AudioGraph &graph) -> AudioError = 0;
  virtual void unlink(AudioGraph &graph) = 0;
  [[nodiscard]] virtual auto map(ModulePort port) const -> AudioEndpoint = 0;

  [[nodiscard]] std::uint8_t ins() const { return ins_; }
  [[nodiscard]] std::uint8_t outs() const { return outs_; }

private:
  std::uint8_t ins_;
  std::uint8_t outs_;
};

inline auto connect(const Patchable &src, ModulePort src_port,
                    const Patchable &dst, ModulePort dst_port,
                    AudioGraph &graph) {
  if (src_port.index >= src.outs() || src_port.index >= dst.ins()) {
    return AudioError::INVALID_PORT;
  }
  const auto src_endpt = src.map(src_port);
  const auto dst_endpt = dst.map(dst_port);
  return graph.connect(src_endpt.id, src_endpt.port, dst_endpt.id,
                       dst_endpt.port);
}

inline auto disconnect(const Patchable &src, ModulePort src_port,
                       const Patchable &dst, ModulePort dst_port,
                       AudioGraph &graph) {
  if (src_port.index >= src.outs() || src_port.index >= dst.ins()) {
    return AudioError::INVALID_PORT;
  }
  const auto src_endpt = src.map(src_port);
  const auto dst_endpt = dst.map(dst_port);
  return graph.disconnect(src_endpt.id, src_endpt.port, dst_endpt.id,
                          dst_endpt.port);
}

inline auto connect(AudioEndpoint src, AudioEndpoint dst, AudioGraph &graph) {
  return graph.connect(src.id, src.port, dst.id, dst.port);
}

inline auto disconnect(AudioEndpoint src, AudioEndpoint dst,
                       AudioGraph &graph) {
  return graph.disconnect(src.id, src.port, dst.id, dst.port);
}

[[nodiscard]] inline float processor_usage(AudioGraph &graph) {
  float sum{};
  for (const auto &n : graph.nodes()) {
    sum += n.device->processorUsage();
  }
  return sum;
}

[[nodiscard]] inline float processor_usage_max(AudioGraph &graph) {
  float sum{};
  for (const auto &n : graph.nodes()) {
    sum += n.device->processorUsageMax();
  }
  return sum;
}

inline void processor_usage_max_reset(AudioGraph &graph) {
  for (const auto &n : graph.nodes()) {
    n.device->processorUsageMaxReset();
  }
}

} // namespace sndbx::audio

namespace sndbx {
template <typename M> struct StaticModuleInfo {
  nst::inplace_vector<std::string_view, 4> ctrls;
  nst::inplace_vector<std::string_view, 8> ins;
  nst::inplace_vector<std::string_view, 8> outs;
};

} // namespace sndbx