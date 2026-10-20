
#include "oscillator.hpp"
#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"

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

void Oscillator::unlink(AudioGraph &graph) { graph.remove_node(synth_id_); }

AudioEndpoint Oscillator::endpoint(ModulePort port) const {
	return audio::make_endpoint(synth_id_, port.index);
}

float Oscillator::change_control(ControlChange c) {
	assert(c.index < num_controls());

	switch (c.index) {
	case 0:
		return ratio(coarse_tune_ *= powf(1.08f, c.amt));
	case 1:
		return ratio(fine_tune_ += c.amt);
	case 2:
		return ratio(fm_ctrl_ += 0.25f * c.amt);
	case 3:
		return ratio(waveform_ctrl_ += c.amt);
	}
	return 0;
}

ColoredText Oscillator::display_text() const {
	const auto &wave = osc_waveforms[waveform_idx_];
	return {wave.name, wave.color};
}

nst::teensy::ColorRGB Oscillator::led_color() const {
	return osc_waveforms[waveform_idx_].color;
}

void Oscillator::set_defaults() {
	frequency_ = 440.0f;
	fine_tune_ = 0;
	fm_depth_ = 8.25f;
	waveform_idx_ = 0;
}

void Oscillator::update_frequency() {
	synth_.frequency(frequency_ + fine_tune_offset_);
}

void Oscillator::update_fm_depth() { synth_.frequencyModulation(fm_depth_); }

void Oscillator::update_waveform() {
	synth_.begin(osc_waveforms[waveform_idx_].id);
}

void Oscillator::init_synth() {
	update_fm_depth();
	update_frequency();
	update_waveform();
}

CtrlFmtStr waveform_ctrl_formatter(float f) {
	return CtrlFmtStr{osc_waveforms[f * osc_waveforms.max_size()].name};
}

} // namespace sndbx