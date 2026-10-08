#ifndef SANDBOX_AUDIO_ENGINE_HPP_
#define SANDBOX_AUDIO_ENGINE_HPP_

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <utility>

#include "audio/audio_trigger.hpp"
#include "config/config.hpp"
#include "modules/parameter.hpp"
#include <nst/audio_graph.hpp>

namespace sndbx {

using AudioGraph = nst::teensy::AudioGraph<limits::max_audio_graph_nodes,
                                           limits::max_audio_graph_patches>;

using AudioError = nst::teensy::AudioGraphError;

struct AudioEndpoint {
	nst::teensy::AudioNodeID id;
	nst::teensy::AudioPort port;
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

// Interface between a module and the lower-level AudioGraph.
class Patchable {
  public:
	Patchable(std::uint8_t ins, std::uint8_t outs) : ins_{ins}, outs_{outs} {}
	virtual ~Patchable() = default;

	// create the required nodes and/or connections in the audio graph. If any
	// operation fails, undo the previous operations, then return the error
	[[nodiscard]] virtual auto link(AudioGraph &graph) -> AudioError = 0;

	// remove any nodes created in link() from the graph
	virtual void unlink(AudioGraph &graph) = 0;

	// map a module port to an AudioEndpoint.
	// depending on port.type, check that port.index < ins() or < outs() before
	// calling
	[[nodiscard]] virtual auto endpoint(ModulePort port) const
	    -> AudioEndpoint = 0;

	// the number of input ports
	[[nodiscard]] std::uint8_t ins() const { return ins_; }

	// the number of output ports
	[[nodiscard]] std::uint8_t outs() const { return outs_; }

  private:
	std::uint8_t ins_;
	std::uint8_t outs_;
};

// Create an AudioEndpoint from a graph node id and port index
[[nodiscard]] inline auto make_endpoint(nst::teensy::AudioNodeID id,
                                        std::uint8_t graph_port = 0) {
	return AudioEndpoint{id, nst::teensy::AudioPort{graph_port}};
}

// helper function template for mapping a module port to an audio endpoint.
// InputMap and OutputMap model callables that take in a port index and
// return an AudioEndpoint.
template <typename InputMap, typename OutputMap>
[[nodiscard]] AudioEndpoint map_port(ModulePort port, InputMap &&in_map,
                                     OutputMap &&out_map) {
	switch (port.type) {
	case ModulePort::Type::IN:
		return in_map(port.index);
	case ModulePort::Type::OUT:
		return out_map(port.index);
	default:
		return {};
	}
}

// connect two Patchables by port index
inline AudioError connect(const Patchable &src, std::uint8_t output_idx,
                          const Patchable &dst, std::uint8_t input_idx,
                          AudioGraph &graph) {
	if (output_idx >= src.outs() || input_idx >= dst.ins()) {
		return AudioError::INVALID_PORT;
	}
	const auto [src_node_id, src_node_port] =
	    src.endpoint(ModulePort{ModulePort::Type::OUT, output_idx});
	const auto [dst_node_id, dst_node_port] =
	    dst.endpoint(ModulePort{ModulePort::Type::IN, output_idx});
        
	return graph.connect(src_node_id, src_node_port, dst_node_id,
	                        dst_node_port);
}

// disconnect two Patchables by port index
inline AudioError disconnect(const Patchable &src, std::uint8_t output_idx,
                             const Patchable &dst, std::uint8_t input_idx,
                             AudioGraph &graph) {
	if (output_idx >= src.outs() || input_idx >= dst.ins()) {
		return AudioError::INVALID_PORT;
	}
	const auto [src_node_id, src_node_port] =
	    src.endpoint(ModulePort{ModulePort::Type::OUT, output_idx});
	const auto [dst_node_id, dst_node_port] =
	    dst.endpoint(ModulePort{ModulePort::Type::IN, output_idx});

	return graph.disconnect(src_node_id, src_node_port, dst_node_id,
	                        dst_node_port);
}

// Query AudioGraph processor usage
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

#endif