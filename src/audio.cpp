#include "audio.hpp"

namespace sndbx::audio {

void allocate_blocks() { AudioMemory(limits::max_audio_blocks); }

inline AudioPoint no_inputs(std::uint8_t) { nst::unreachable(); }
inline AudioPoint no_outputs(std::uint8_t) { nst::unreachable(); }

bool register_nodes(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioNodeID &>>
        node_data) {
	int nodes_added{};
	for (auto &data : node_data) {
		const auto result = graph.add(data.first);
		if (!result) {
			// remove previous successes if failure
			std::for_each_n(node_data.begin(), nodes_added,
			                [&graph](auto pair) { graph.remove(pair.second); });
			return false;
		}
		data.second = *result;
		++nodes_added;
	}
	return true;
}

bool register_nodes(
    AudioGraph &graph,
    std::pair<AudioStream *, nst::teensy::AudioNodeID &> node_data) {
	const auto id = graph.add(node_data.first);
	if (!id) {
		return false;
	}
	node_data.second = *id;
	return true;
}

} // namespace sndbx::audio