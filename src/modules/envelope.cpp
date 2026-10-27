#include "envelope.hpp"

namespace sndbx {

AudioError Envelope::link(AudioGraph &graph) {
	return audio::add_nodes(graph, {{&env_, env_id_}, {&trig_, trig_id_}});
}

void Envelope::unlink(AudioGraph &graph) {
	graph.remove(env_id_);
	graph.remove(trig_id_);
}

AudioEndpoint Envelope::patch_point(ModulePort port) const {
	using namespace audio;

	return map_port(
	    port,
	    [this](auto in) {
		    return in == 0 ? make_endpoint(env_id_, 0)
		                   : make_endpoint(trig_id_, 0);
	    },
	    [this](auto out) { return make_endpoint(env_id_, 0); });
}

float Envelope::change_control(ControlChange c) {
	assert(c.index < num_controls());
	switch (c.index) {
	case 0:
		return attack_ctrl.change(*this, attack_ms_, c.delta);
	case 1:
		return decay_ctrl.change(*this, decay_ms_, c.delta);
	case 2:
		return sustain_ctrl.change(*this, sustain_ms_, c.delta);
	case 3:
		return release_ctrl.change(*this, release_ms_, c.delta);
	}
	return 0;
}

const ModuleDisplayInfo &Envelope::display_info() const {
	return module_info<Self>;
}

ColoredText Envelope::display_text() const { return {"envelope", 0xFFFF00}; }
nst::teensy::ColorRGB Envelope::led_color() const { return 0xFFFF00; }

void Envelope::trigger(AudioEdge e) {
	switch (e) {
	case AudioEdge::RISING_EDGE:
		env_.noteOn();
		break;
	case AudioEdge::FALLING_EDGE:
		env_.noteOff();
		break;
	}
}

void Envelope::on_rising_edge() { trigger(AudioEdge::RISING_EDGE); }

void Envelope::on_falling_edge() { trigger(AudioEdge::FALLING_EDGE); }

void Envelope::update_attack() { env_.attack(attack_ms_); }

void Envelope::update_decay() { env_.decay(decay_ms_); }

void Envelope::update_sustain() { env_.sustain(sustain_ms_); }

void Envelope::update_release() { env_.release(release_ms_); }

} // namespace sndbx