#include "audio_out.hpp"
#include "audio.hpp"

namespace sndbx {

bool AudioOut::link(AudioGraph &graph) {
	auto stereo_connect = [&graph](auto src, auto dst) {
		return graph.connect(src, dst) && graph.connect(src, 0, dst, 1);
	};

	return audio::register_devices(graph, {{&amp_, amp_id_},
	                                       {&usb_out_, usb_id_},
	                                       {&i2s_out_, i2s_id_}}) &&
	       stereo_connect(amp_id_, i2s_id_) && stereo_connect(amp_id_, usb_id_);
}

AudioPoint AudioOut::patch_point(ModulePort port) const {
	using namespace audio;
	return map_module_port(
	    port,
	    [this](auto in) {
		    assert(in == 0);
		    return make_patch_point(amp_id_);
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