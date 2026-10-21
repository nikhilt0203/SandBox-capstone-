#ifndef SANDBOX_OSCILLATOR_HPP_
#define SANDBOX_OSCILLATOR_HPP_

#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"

namespace sndbx {
class Oscillator;

CtrlFmtStr waveform_ctrl_formatter(float f);

template <>
inline constexpr ModuleDisplayInfo module_info<Oscillator>{
    {"oscillator", 0x00FF00},
    "outputs a continous waveform modulated by an fm input",
    {"coarse", "fine", "fm", "type"},
    {"in", "fm"},
    {"out"},
    {ctrl_formatter<HertzFormat>, ctrl_formatter<HertzFormat>,
     ctrl_formatter<OctavesFormat>, waveform_ctrl_formatter}};

class Oscillator : public audio::Patchable,
                   public Controllable,
                   public Displayable {
	using Self = Oscillator;

  public:
	Oscillator()
	    : Patchable{num_ins<Self>, num_outs<Self>},
	      Controllable{num_ctrls<Self>} {
		set_defaults();
	}

	[[nodiscard]] AudioError link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	[[nodiscard]] AudioEndpoint endpoint(ModulePort port) const override;

	float change_control(ControlChange c) override;

	[[nodiscard]] const ModuleDisplayInfo &display_info() const override {
		return module_info<Self>;
	}

	[[nodiscard]] ColoredText display_text() const override;
	[[nodiscard]] nst::teensy::ColorRGB led_color() const override;

  protected:
	void init_synth();
	void set_defaults();

	void update_frequency();
	void update_fm_depth();
	void update_waveform();

	AudioSynthWaveformModulated synth_;
	nst::teensy::AudioNodeID synth_id_;

	ModuleParameter<float> frequency_{0.01f, 18000.0f, 440.0f};
	ModuleParameter<int> fine_tune_offset_{-50, 50, 0};
	ModuleParameter<float> fm_depth_{0.0f, 12.0f, 8.25f};
	ModuleParameter<std::size_t, WrapAtBounds> waveform_idx_{0, 6, 0};

	struct FmCoeff {
		constexpr static float value{0.25};
	};

	constexpr static ModuleControl<Self, decltype(frequency_)> coarse_tune_{
	    [](float v, int x) { return v * powf(1.08f, x); },
	    &Self::update_frequency};

	constexpr static ModuleControl<Self, decltype(fine_tune_offset_)>
	    fine_tune_{linear_curve<int>, &Self::update_frequency};

	constexpr static ModuleControl<Self, decltype(fm_depth_)> fm_ctrl_{
	    linear_curve<float, FmCoeff>, &Self::update_fm_depth};

	constexpr static ModuleControl<Self, decltype(waveform_idx_)>
	    waveform_ctrl_{linear_curve<std::size_t>, &Self::update_waveform};

};
} // namespace sndbx

#endif