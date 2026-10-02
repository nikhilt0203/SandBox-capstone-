#ifndef SANDBOX_OSCILLATOR_HPP_
#define SANDBOX_OSCILLATOR_HPP_

#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "parameter.hpp"

namespace sndbx {

class Oscillator;

template <>
constexpr ModuleDisplayInfo module_info<Oscillator>{
    {"oscillator", 0x00FF00},
    "outputs a continous waveform modulated by an fm input",
    {"in", "fm"},
    {"out"},
    {"coarse", "fine", "fm", "type"}};

class Oscillator : public audio::Patchable,
                   public Controllable,
                   public Displayable {
  public:
	Oscillator() : Patchable{2, 1}, Controllable{4} { set_defaults(); }

	[[nodiscard]] AudioError link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;

	[[nodiscard]] AudioEndpoint map(ModulePort port) const override;

	std::uint8_t change_control(std::uint8_t idx, std::int8_t amt) override;

	[[nodiscard]] const ModuleDisplayInfo &display_info() const override;
	[[nodiscard]] ColoredText display_text() const override;
	[[nodiscard]] nst::teensy::ColorRGB led_color() const override;

  protected:
	void init_synth();
	void set_defaults();
	void coarse_tune(int amt);
	void fine_tune(int amt);
	void fm_adjust(int amt);
	void set_waveform(std::size_t waveform);
	void change_waveform(int amt);

	ModuleParameter<float> frequency_{0.01f, 18000.0f};
	ModuleParameter<int> fine_tune_{-50, 50};
	ModuleParameter<float> fm_depth_{0.0f, 12.0f};
	ModuleParameter<std::size_t> waveform_idx_{0U, 6U};

	AudioSynthWaveformModulated synth_;
	nst::teensy::AudioNodeID synth_id_;
};

} // namespace sndbx

#endif