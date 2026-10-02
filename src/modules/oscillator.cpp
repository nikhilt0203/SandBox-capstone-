
#include "oscillator.hpp"
#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "parameter.hpp"

namespace sndbx {

namespace {
struct OscWaveform {
	constexpr OscWaveform(short id, std::string_view name, std::uint32_t color)
	    : id{id}, name{name}, color{color} {}

	short id;
	std::string_view name;
	std::uint32_t color;
};

static constexpr std::array<OscWaveform, 7> osc_waveforms = {
    OscWaveform{WAVEFORM_SINE, "sine", 0x0FF00},
    OscWaveform{WAVEFORM_SQUARE, "square", 0xFF000},
    OscWaveform{WAVEFORM_SAWTOOTH, "saw", 0xFF00F},
    OscWaveform{WAVEFORM_TRIANGLE, "triangle", 0xFFF00},
    OscWaveform{WAVEFORM_PULSE, "pulse", 0x00832},
    OscWaveform{WAVEFORM_SAWTOOTH_REVERSE, "rev saw", 0xB817E},
    OscWaveform{WAVEFORM_SAMPLE_HOLD, "s&h noise", 0x09F77}};
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

std::uint8_t Oscillator::change_control(std::uint8_t idx, std::int8_t amt) {
	assert(idx < num_ctrls());

	switch (idx) {
	case 0:
		coarse_tune(amt);
		return scale_to<std::uint8_t>(frequency_);
	case 1:
		fine_tune(amt);
		return scale_to<std::uint8_t>(fine_tune_);
	case 2:
		fm_adjust(amt);
		return scale_to<std::uint8_t>(fm_depth_);
	case 3:
		change_waveform(amt);
		return scale_to<std::uint8_t>(waveform_idx_);
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