#include "audio_out.hpp"

namespace sndbx {

bool AudioOut::link(AudioGraph &graph) {
	return audio::register_nodes(graph, {{&amp_, amp_id_},
	                                     {&usb_out_, usb_id_},
	                                     {&i2s_out_, i2s_id_}}) &&
	       graph.connect(amp_id_, i2s_id_) && graph.connect(amp_id_, usb_id_);
}

AudioPoint AudioOut::patch_point(ModulePort port) const {
	using namespace audio;
	return map_port(
	    port,
	    [this](auto in) {
		    assert(in == 0);
		    return make_audio_point(amp_id_, 0);
	    },
	    no_outputs);
}

void AudioOut::unlink(AudioGraph &graph) {
	graph.remove(amp_id_);
	graph.remove(i2s_id_);
	graph.remove(usb_id_);
}

float AudioOut::change_control(ControlChange cc) {
	assert(cc.index == 0);
	return gain_ctrl.change(*this, gain_, cc.delta);
}

const ModuleDisplayInfo &AudioOut::display_info() const {
	return module_info<AudioOut>;
}

void AudioOut::update_gain() { amp_.gain(gain_); }

} // namespace sndbx