#include "mixer.hpp"

namespace sndbx {

bool Mixer::link(AudioGraph &graph) {
	return audio::register_nodes(graph, {&mix_, mix_id_});
}

void Mixer::unlink(AudioGraph &graph) { graph.remove(mix_id_); }

AudioPoint Mixer::patch_point(ModulePort port) const {
	return audio::make_audio_point(mix_id_, port.index);
}

float Mixer::change_control(ControlChange c) {
	assert(c.index < num_controls());
	return gain_ctrl.change(*this, channel_gains_[c.index], c.delta);
}

const ModuleDisplayInfo &Mixer::display_info() const {
	return module_info<Self>;
}

void Mixer::update_gains() {
	for (std::size_t ch{}; ch < channel_gains_.size(); ++ch) {
		mix_.gain(ch, channel_gains_[ch]);
	}
}

} // namespace sndbx
