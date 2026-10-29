#ifndef SANDBOX_AUDIO_ENGINE_HPP_
#define SANDBOX_AUDIO_ENGINE_HPP_

#include <algorithm>
#include <string_view>
#include <utility>

#include <nst/teensy/audio_graph.hpp>
#include <nst/unreachable.hpp>

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

enum class PortType { IN, OUT };

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

void init();

// Invalid AudioEndpoint for unreachable return paths
inline constexpr AudioEndpoint null_endpoint{nst::teensy::AudioNodeID{-1},
                                             nst::teensy::AudioPort{0}};

// Create an AudioEndpoint from a graph node id and port index
[[nodiscard]] inline auto make_endpoint(nst::teensy::AudioNodeID id,
                                        std::uint8_t audio_port = 0) {
	return AudioEndpoint{id, nst::teensy::AudioPort{audio_port}};
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
	}
	nst::unreachable{};
	return null_endpoint;
}

// Takes a initializer list of AudioStream * and AudioNodeID & (out param)
// pairs, adds each device to the audio graph and writes the id to the
// corresponding out param. If any node addition fails, all nodes are removed
// and the error is returned.
[[nodiscard]] inline AudioError add_nodes(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioNodeID &>>
        node_data) {
	int successes{};
	for (auto &data : node_data) {
		auto device = data.first;
		assert(device);
		const auto id = graph.add(device);
		if (!id) {
			std::for_each_n(
			    node_data.begin(), successes,
			    [&graph](const auto &pair) { graph.remove(pair.second); });
			return id.error();
		}
		data.second = *id;
		++successes;
	}
	return AudioError::NONE;
}

// Overload for adding only one node
[[nodiscard]] inline auto
add_nodes(AudioGraph &graph,
          std::pair<AudioStream *, nst::teensy::AudioNodeID &> node_data) {
	return add_nodes(graph, {node_data});
}

} // namespace sndbx::audio

#endif