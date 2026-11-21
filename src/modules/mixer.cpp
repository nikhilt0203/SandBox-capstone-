#include "mixer.hpp"

namespace sndbx {

bool Mixer::link(AudioGraph &graph) {
	return audio::register_devices(graph, {&mix_, mix_id_});
}

void Mixer::unlink(AudioGraph &graph) { graph.remove(mix_id_); }

AudioPoint Mixer::patch_point(ModulePort port) const {
	return audio::make_audio_point(mix_id_, port.index);
}

float Mixer::change_control(ControlChange cc) {
	return gain_ctrl.change(*this, channel_gains_[cc.index], cc.delta);
}

const ModuleDisplayInfo &Mixer::display_info() const {
	return module_info<Mixer>;
}

void Mixer::update_gains() {
	for (std::size_t ch{}; ch < channel_gains_.size(); ++ch) {
		mix_.gain(ch, channel_gains_[ch]);
	}
}

} // namespace sndbx
