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
    {"out"},
    {ctrl_format<FMillis>, ctrl_format<FMillis>, ctrl_format<FMillis>,
     ctrl_format<FMillis>}};

class Envelope : public Patchable,
                 public Controllable,
                 public Displayable,
                 public Pressable {
	using Self = Envelope;

  public:
	Envelope()
	    : Patchable{num_ins<Self>, num_outs<Self>},
	      Controllable{num_ctrls<Self>} {}

	AudioError link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	AudioEndpoint patch_point(ModulePort port) const override;

	float change_control(ControlChange c) override;

	const ModuleDisplayInfo &display_info() const override;

	ColoredText display_text() const override;
	nst::teensy::ColorRGB led_color() const override;

	void trigger(AudioEdge e);

	void on_rising_edge() override;
	void on_falling_edge() override;

  private:
	void update_attack();
	void update_decay();
	void update_sustain();
	void update_release();

	AudioEffectEnvelope env_;
	AudioTriggerInput<Envelope> trig_{*this};
	nst::teensy::AudioNodeID env_id_;
	nst::teensy::AudioNodeID trig_id_;

	using Parameter = ModuleParameter<float>;

	Parameter attack_ms_{0.0f, 500.0f, 10.0f};
	Parameter decay_ms_{0.0f, 500.0f, 35.0f};
	Parameter sustain_ms_{0.0f, 1.0f, 0.0f};
	Parameter release_ms_{0.0f, 500.0f, 100.0f};

	struct sustain_coeff {
		constexpr static auto value{0.25f};
	};

	constexpr static auto ms_curve = curves::linear<float, 5>;
	constexpr static auto sustain_curve = curves::linear<float, sustain_coeff>;

	constexpr static auto attack_ctrl =
	    ModuleControl{ms_curve, &Self::update_attack};

	constexpr static auto decay_ctrl =
	    ModuleControl{ms_curve, &Self::update_decay};

	constexpr static auto sustain_ctrl =
	    ModuleControl{sustain_curve, &Self::update_sustain};

	constexpr static auto release_ctrl =
	    ModuleControl{ms_curve, &Self::update_release};
};

} // namespace sndbx

#endif