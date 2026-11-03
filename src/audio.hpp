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

struct AudioPoint {
	nst::teensy::AudioNodeID id;
	nst::teensy::AudioPort port;
};

// Keeps the compiler happy in unreachable paths that return an AudioPoint
inline constexpr auto null_audio_point =
    AudioPoint{nst::teensy::AudioNodeID{-1}, nst::teensy::AudioPort{0}};

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

namespace audio {

void allocate_blocks();

// Denotes that a module has no input ports in map_port()
inline AudioPoint no_inputs(std::uint8_t) { nst::unreachable(); }

// Denotes that a module has no output ports in map_port()
inline AudioPoint no_outputs(std::uint8_t) { nst::unreachable(); }

// Create an AudioPoint from a graph node id and port index
[[nodiscard]] inline constexpr auto
make_audio_point(nst::teensy::AudioNodeID id, std::uint8_t audio_port = 0) {
	return AudioPoint{id, nst::teensy::AudioPort{audio_port}};
}

// Registers multiple AudioStream objects with the AudioGraph. Supplied
// as a list of pairs of an AudioStream * and AudioNodeID out param. If any
// addition in the list fails, all nodes in the list are removed from the graph.
[[nodiscard]] bool register_nodes(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioNodeID &>>
        node_data);

// Overload for adding only one node
[[nodiscard]] bool
register_nodes(AudioGraph &graph,
               std::pair<AudioStream *, nst::teensy::AudioNodeID &> node_data);

// helper function template for mapping a module port to an audio patch_point.
// InputMap and OutputMap model callables that take in a port index and
// return an AudioPoint.
template <typename InputMapping, typename OutputMapping>
[[nodiscard]] AudioPoint map_port(ModulePort port, InputMapping &&in_map_func,
                                  OutputMapping &&out_map_func) {
	switch (port.type) {
	case PortType::IN:
		return std::forward<InputMapping>(in_map_func)(port.index);
	case PortType::OUT:
		return std::forward<OutputMapping>(out_map_func)(port.index);
	}
	nst::unreachable();
}

} // namespace audio
} // namespace sndbx

#endif