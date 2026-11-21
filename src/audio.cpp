#include "audio.hpp"

AudioControlSGTL5000 audio_shield;

namespace sndbx::audio {

void allocate_blocks() { AudioMemory(limits::max_audio_blocks); }

void enable_audio_shield(float volume) {
	audio_shield.enable();
	audio_shield.volume(volume);
}

bool register_devices(
    AudioGraph &graph,
    std::initializer_list<std::pair<AudioStream *, nst::teensy::AudioID &>>
        device_data) {
	int num_added{};
	for (auto &data : device_data) {
		const auto result = graph.add(data.first);
		if (!result) {
			// remove previous successes if failure
			std::for_each_n(device_data.begin(), num_added,
			                [&graph](auto pair) { graph.remove(pair.second); });
			return false;
		}
		data.second = *result;
		++num_added;
	}
	return true;
}

bool register_devices(
    AudioGraph &graph,
    std::pair<AudioStream *, nst::teensy::AudioID &> device_data) {
	const auto id = graph.add(device_data.first);
	if (!id) {
		return false;
	}
	device_data.second = *id;
	return true;
}

} // namespace sndbx::audio