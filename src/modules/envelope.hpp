#ifndef SANDBOX_ENVELOPE_HPP_
#define SANDBOX_ENVELOPE_HPP_

#include "audio/audio_trigger.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"
#include "patchable.hpp"
#include "pressable.hpp"

namespace sndbx {
class Envelope;

template <>
inline constexpr ModuleDisplayInfo module_info<Envelope>{
    {"envelope", 0xFFFF00},
    "applies an envelope to the input signal",
    {"attack", "decay", "sustain", "release"},
    {"in", "trg"},
    {"out"}};

class Envelope : public Patchable,
                 public Controllable,
                 public Displayable,
                 public Pressable {
	using Self = Envelope;

  public:
	Envelope() : Patchable{2, 1}, Controllable{4} {}

	bool link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	AudioPoint patch_point(ModulePort port) const override;

	float change_control(ControlChange cc) override;

	const ModuleDisplayInfo &display_info() const override;

	ColoredText display_text() const override;
	nst::teensy::ColorRGB led_color() const override;

	void on_rising_edge() override;
	void on_falling_edge() override;
	void trigger(AudioEdge e);

  private:
	void update_attack();
	void update_decay();
	void update_sustain();
	void update_release();

	AudioEffectEnvelope env_;
	AudioTriggerInput<Self> trig_{*this};
	nst::teensy::AudioID env_id_;
	nst::teensy::AudioID trig_id_;

	using Parameter = ModuleParameter<float>;

	Parameter attack_ms_{0.0f, 500.0f, 10.0f};
	Parameter decay_ms_{0.0f, 500.0f, 35.0f};
	Parameter sustain_ms_{0.0f, 1.0f, 0.0f};
	Parameter release_ms_{0.0f, 500.0f, 100.0f};

	constexpr static auto ms_curve = curves::linear<float, 5>;
	constexpr static auto sustain_curve =
	    curves::linear<float, nst::ratio<25, 100>>;

	constexpr static ModuleControl attack_ctrl{ms_curve, &Self::update_attack};

	constexpr static ModuleControl decay_ctrl{ms_curve, &Self::update_decay};

	constexpr static ModuleControl sustain_ctrl{sustain_curve,
	                                            &Self::update_sustain};

	constexpr static ModuleControl release_ctrl{ms_curve,
	                                            &Self::update_release};
};

} // namespace sndbx

#endif