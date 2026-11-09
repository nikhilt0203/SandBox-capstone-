#ifndef SANDBOX_OSCILLATOR_HPP_
#define SANDBOX_OSCILLATOR_HPP_

#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"
#include "patchable.hpp"

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
    {ctrl_format<FHertz>, ctrl_format<FHertz>, ctrl_format<FOctaves>,
     waveform_ctrl_formatter}};

class Oscillator : public Patchable, public Controllable, public Displayable {
	using Self = Oscillator;

  public:
	Oscillator() : Patchable{2, 1}, Controllable{4} { init_synth(); }

	bool link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	AudioPoint patch_point(ModulePort port) const override;

	float change_control(ControlChange c) override;

	const ModuleDisplayInfo &display_info() const override {
		return module_info<Self>;
	}

	ColoredText display_text() const override;
	nst::teensy::ColorRGB led_color() const override;

  protected:
	void init_synth();
	void update_frequency();
	void update_fm_depth();
	void update_waveform();

	using fm_coeff = curves::Ratio<25, 100>;

	constexpr static ModuleControl coarse_ctrl{curves::default_freq_curve,
	                                           &Self::update_frequency};

	constexpr static ModuleControl fine_ctrl{curves::linear<int>,
	                                         &Self::update_frequency};

	constexpr static ModuleControl fm_ctrl{curves::linear<float, fm_coeff>,
	                                       &Self::update_fm_depth};

	constexpr static ModuleControl waveform_ctrl{curves::linear<std::size_t>,
	                                             &Self::update_waveform};

	AudioSynthWaveformModulated synth_;
	nst::teensy::AudioNodeID synth_id_;

	ModuleParameter<float> frequency_{0.01f, 18000.0f, 440.0f};
	ModuleParameter<int> fine_tune_offset_{-50, 50, 0};
	ModuleParameter<float> fm_depth_{0.0f, 12.0f, 8.25f};
	ModuleParameter<std::size_t, WrapAtBounds> waveform_idx_{0, 6, 0};
};

} // namespace sndbx

#endif