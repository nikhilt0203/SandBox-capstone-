
#include "oscillator.hpp"
#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "parameter.hpp"

namespace sndbx {

namespace {
struct Waveform {
	constexpr Waveform(std::string_view name, short id,
	                   nst::teensy::ColorRGB color)
	    : id{id}, name{name}, color{color} {}

	short id;
	std::string_view name;
	nst::teensy::ColorRGB color;
};

static constexpr std::array<Waveform, 7> osc_waveforms = {
    Waveform{"sine", WAVEFORM_SINE, 0x0FF00},
    Waveform{"square", WAVEFORM_SQUARE, 0xFF000},
    Waveform{"saw", WAVEFORM_SAWTOOTH, 0xFF00F},
    Waveform{"triangle", WAVEFORM_TRIANGLE, 0xFFF00},
    Waveform{"pulse", WAVEFORM_PULSE, 0x00832},
    Waveform{"rev saw", WAVEFORM_SAWTOOTH_REVERSE, 0xB817E},
    Waveform{"s&h noise", WAVEFORM_SAMPLE_HOLD, 0x09F77}};
} // namespace

AudioError Oscillator::link(AudioGraph &graph) {
	auto id = graph.add_node(&synth_);
	if (!id) {
		return id.error();
	}
	synth_id_ = *id;
	return AudioError::NONE;
}

void Oscillator::unlink(AudioGraph &graph) { graph.remove_node(synth_id_); }

AudioEndpoint Oscillator::map(ModulePort port) const {
	return audio::make_endpoint(synth_id_, 0);
}

float Oscillator::change_control(std::uint8_t idx, std::int8_t amt) {
	assert(idx < num_controls());

	switch (idx) {
	case 0:
		coarse_tune(amt);
		return frequency_.ratio();
	case 1:
		fine_tune(amt);
		return fine_tune_.ratio();
	case 2:
		fm_adjust(amt);
		return fm_depth_.ratio();
	case 3:
		change_waveform(amt);
		return waveform_idx_.ratio();
	}
	return 0;
}

ColoredText Oscillator::display_text() const {
	const auto &wave = osc_waveforms[waveform_idx_];
	return {wave.name, wave.color};
}

void Oscillator::set_defaults() {
	frequency_ = 440.0f;
	fine_tune_ = 0;
	fm_depth_ = 8.25f;
	waveform_idx_ = 0;
}

void Oscillator::coarse_tune(int amt) {
	frequency_ *= powf(1.08f, amt);
	synth_.frequency(frequency_ + fine_tune_);
}

void Oscillator::fine_tune(int amt) {
	fine_tune_ += amt;
	synth_.frequency(frequency_ + fine_tune_);
}

void Oscillator::fm_adjust(int amt) {
	fm_depth_ += 0.25f * amt;
	synth_.frequencyModulation(fm_depth_);
}

void Oscillator::set_waveform(std::size_t waveform) {
	waveform_idx_ = waveform;
	synth_.begin(osc_waveforms[waveform_idx_].id);
}

void Oscillator::change_waveform(int amt) {
	int total = osc_waveforms.size();
	const auto wrapped_idx =
	    ((static_cast<int>(waveform_idx_) + amt) % total + total) % total;
	set_waveform(wrapped_idx);
}

void Oscillator::init_synth() {
	synth_.frequency(frequency_);
	synth_.frequencyModulation(fm_depth_);
	set_waveform(waveform_idx_);
}

} // namespace sndbx