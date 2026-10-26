#ifndef SANDBOX_AUDIO_ENGINE_HPP_
#define SANDBOX_AUDIO_ENGINE_HPP_

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

#include <nst/audio_graph.hpp>

#include "audio/audio_trigger.hpp"
#include "config/config.hpp"

namespace sndbx {

using AudioGraph = nst::teensy::AudioGraph<limits::max_audio_graph_nodes,
                                           limits::max_audio_graph_patches>;

using AudioError = nst::teensy::AudioGraphError;

[[nodiscard]] inline constexpr bool is_success(AudioError e) {
	return e == AudioError::NONE;
}

struct AudioEndpoint {
	nst::teensy::AudioNodeID id;
	nst::teensy::AudioPort port;
};

enum class PortType : std::uint8_t { IN, OUT };

struct ModulePort {
	PortType type;
	std::uint8_t index{};

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

	// map a module port to an AudioEndpoint
	[[nodiscard]] virtual auto patch_point(ModulePort port) const
	    -> AudioEndpoint = 0;

	[[nodiscard]] std::uint8_t ins() const { return ins_; }
	[[nodiscard]] std::uint8_t outs() const { return outs_; }

  private:
	std::uint8_t ins_;
	std::uint8_t outs_;
};

// Create an AudioEndpoint from a graph node id and port index
[[nodiscard]] inline auto make_patch_point(nst::teensy::AudioNodeID id,
                                           std::uint8_t graph_port) {
	return AudioEndpoint{id, nst::teensy::AudioPort{graph_port}};
}

// helper function template for mapping a module port to an audio patch_point.
// InputMap and OutputMap model callables that take in a port index and
// return an AudioEndpoint.
template <typename InputMap, typename OutputMap>
[[nodiscard]] AudioEndpoint map_port(ModulePort port, InputMap &&in_map,
                                     OutputMap &&out_map) {
	switch (port.type) {
	case PortType::IN:
		return std::forward<InputMap>(in_map)(port.index);
	case PortType::OUT:
		return std::forward<OutputMap>(out_map)(port.index);
	default:
		assert(false && "Invalid port type");
		return {};
	}
}

// Takes a initializer list of AudioStream * and AudioNodeID & (out param)
// pairs, adds each device to the audio graph and writes the id to the
// corresponding out param. If any node addition fails, all nodes are removed
// and the error is returned.
[[nodiscard]] inline AudioError add_nodes(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioNodeID &>>
        node_data) {
	int nodes_added{};
	for (auto &data : node_data) {
		auto device = data.first;
		assert(device);
		const auto id = graph.add(device);
		if (!id) {
			std::for_each_n(
			    node_data.begin(), nodes_added,
			    [&graph](const auto &pair) { graph.remove(pair.second); });
			return id.error();
		}
		data.second = *id;
		++nodes_added;
	}
	return AudioError::NONE;
}

// Overload for adding only one node
[[nodiscard]] inline auto
add_nodes(AudioGraph &graph,
          std::pair<AudioStream *, nst::teensy::AudioNodeID &> node_data) {
	return add_nodes(graph, {node_data});
}

// connect two Patchables by port index
inline AudioError connect(const Patchable &src, std::uint8_t output_idx,
                          const Patchable &dst, std::uint8_t input_idx,
                          AudioGraph &graph) {
	assert(output_idx >= src.outs() || input_idx >= dst.ins());

	const auto [out_id, out_port] =
	    src.patch_point({PortType::OUT, output_idx});
	const auto [in_id, in_port] = dst.patch_point({PortType::IN, input_idx});

	return graph.connect(out_id, out_port, in_id, in_port);
}

// disconnect two Patchables by port index
inline bool disconnect(const Patchable &src, std::uint8_t output_idx,
                       const Patchable &dst, std::uint8_t input_idx,
                       AudioGraph &graph) {
	assert(output_idx >= src.outs() || input_idx >= dst.ins());

	const auto [out_id, out_port] =
	    src.patch_point({PortType::OUT, output_idx});
	const auto [in_id, in_port] = dst.patch_point({PortType::IN, input_idx});

	return graph.disconnect(out_id, out_port, in_id, in_port);
}

} // namespace sndbx::audio

#endif