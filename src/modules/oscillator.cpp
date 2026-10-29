
#include "oscillator.hpp"
#include "audio.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include <nst/audio_guard.hpp>

namespace {
struct Waveform {
	constexpr Waveform(std::string_view name, short id,
	                   nst::teensy::ColorRGB color)
	    : id{id}, name{name}, color{color} {}

	short id;
	std::string_view name;
	nst::teensy::ColorRGB color;
};

inline constexpr std::array<Waveform, 7> osc_waveforms = {
    Waveform{"sine", WAVEFORM_SINE, 0x0FF00},
    Waveform{"square", WAVEFORM_SQUARE, 0xFF000},
    Waveform{"saw", WAVEFORM_SAWTOOTH, 0xFF00F},
    Waveform{"triangle", WAVEFORM_TRIANGLE, 0xFFF00},
    Waveform{"pulse", WAVEFORM_PULSE, 0x00832},
    Waveform{"rev saw", WAVEFORM_SAWTOOTH_REVERSE, 0xB817E},
    Waveform{"s&h noise", WAVEFORM_SAMPLE_HOLD, 0x09F77}};

} // namespace

namespace sndbx {

AudioError Oscillator::link(AudioGraph &graph) {
	return audio::add_nodes(graph, {&synth_, synth_id_});
}

void Oscillator::unlink(AudioGraph &graph) { graph.remove(synth_id_); }

AudioEndpoint Oscillator::patch_point(ModulePort port) const {
	assert(port.index == 0);
	return audio::make_endpoint(synth_id_, 0);
}

float Oscillator::change_control(ControlChange cc) {
	switch (cc.index) {
	case 0:
		return coarse_ctrl.change(*this, frequency_, cc.delta);
	case 1:
		return fine_ctrl.change(*this, fine_tune_offset_, cc.delta);
	case 2:
		return fm_ctrl.change(*this, fm_depth_, cc.delta);
	case 3:
		return waveform_ctrl.change(*this, waveform_idx_, cc.delta);
	}
	nst::unreachable{};
	return 0;
}

ColoredText Oscillator::display_text() const {
	const auto &wave = osc_waveforms[waveform_idx_];
	return {wave.name, wave.color};
}

nst::teensy::ColorRGB Oscillator::led_color() const {
	return osc_waveforms[waveform_idx_].color;
}

void Oscillator::update_frequency() {
	synth_.frequency(frequency_ + fine_tune_offset_);
}

void Oscillator::update_fm_depth() { synth_.frequencyModulation(fm_depth_); }

void Oscillator::update_waveform() {
	synth_.begin(osc_waveforms[waveform_idx_].id);
}

void Oscillator::init_synth() {
	nst::teensy::AudioGuard lock{};
	synth_.begin(osc_waveforms[waveform_idx_].id);
	synth_.amplitude(0.5);
	update_frequency();
	update_fm_depth();
}

CtrlFmtStr waveform_ctrl_formatter(float f) {
	return CtrlFmtStr{osc_waveforms[f * osc_waveforms.max_size()].name};
}

} // namespace sndbx