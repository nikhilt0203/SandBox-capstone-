#ifndef SANDBOX_AUDIO_ENGINE_HPP_
#define SANDBOX_AUDIO_ENGINE_HPP_

#include "config/config.hpp"
#include <nst/teensy/audio_graph.hpp>
#include <nst/unreachable.hpp>

namespace sndbx {

using AudioGraph = nst::teensy::AudioGraph<limits::max_audio_graph_nodes,
                                           limits::max_audio_graph_patches>;

struct AudioPoint {
	nst::teensy::AudioID id;
	std::uint8_t port;
};

// Keeps the compiler happy in unreachable paths that return an AudioPoint
inline constexpr auto null_audio_point = AudioPoint{nst::teensy::AudioID{0}, 0};

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
void enable_audio_shield(float volume);

// An input mapping function to denote that a module has NO input ports. Should
// not be called!
inline AudioPoint no_inputs(std::uint8_t) {
	// if you reached here, you passed in a bad module port
	nst::unreachable();
}

// An output mapping function to denote that a module has NO output ports.
// Should not be called!
inline AudioPoint no_outputs(std::uint8_t) {
	// if you reached here, you passed in a bad module port
	nst::unreachable();
}

// Create an AudioPoint from a graph node id and port index
[[nodiscard]] inline constexpr auto
make_patch_point(nst::teensy::AudioID device_id, std::uint8_t device_port = 0) {
	return AudioPoint{device_id, device_port};
}

// Registers multiple AudioStream objects with the AudioGraph. Supplied
// as a list of pairs of an AudioStream * and AudioID out param. If any
// addition in the list fails, all nodes in the list are removed from the graph.
[[nodiscard]] bool register_devices(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioID &>>
        device_data);

// Overload for adding only one node
[[nodiscard]] bool
register_devices(AudioGraph &graph,
                 std::pair<AudioStream *, nst::teensy::AudioID &> device_data);

// helper function template for mapping a module port to an AudioPoint.
// InputMapping and OutputMapping model callables that take in a port index and
// return an AudioPoint.
template <typename InputMapping, typename OutputMapping>
[[nodiscard]] AudioPoint map_module_port(ModulePort port,
                                         InputMapping &&in_map_func,
                                         OutputMapping &&out_map_func) {
	switch (port.type) {
	case PortType::IN:
		return std::forward<InputMapping>(in_map_func)(port.index);
	case PortType::OUT:
		return std::forward<OutputMapping>(out_map_func)(port.index);
	}
	nst::unreachable();
}

inline bool connect(AudioPoint src, AudioPoint dst, AudioGraph &graph) {
	return graph.connect(src.id, src.port, dst.id, dst.port);
}

inline bool disconnect(AudioPoint src, AudioPoint dst, AudioGraph &graph) {
	return graph.disconnect(src.id, src.port, dst.id, dst.port);
}

} // namespace audio
} // namespace sndbx

#endif