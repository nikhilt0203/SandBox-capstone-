#ifndef SANDBOX_ENVELOPE_HPP_
#define SANDBOX_ENVELOPE_HPP_

#include "audio/audio_engine.hpp"
#include "audio/audio_trigger.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"
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
    {ctrl_formatter<MillisFormat>, ctrl_formatter<MillisFormat>,
     ctrl_formatter<MillisFormat>, ctrl_formatter<MillisFormat>}};

class Envelope : public audio::Patchable,
                 public Controllable,
                 public Displayable,
                 public Pressable {
	using Self = Envelope;

  public:
	Envelope()
	    : Patchable{num_ins<Self>, num_outs<Self>},
	      Controllable{num_ctrls<Self>} {}

	void add_node(AudioGraph &graph, AudioStream *dev) {}

	[[nodiscard]] AudioError link(AudioGraph &graph) override {
		return audio::add_nodes(graph, {{&env_, env_id_}, {&trig_, trig_id_}});
	}

	void unlink(AudioGraph &graph) override {
		graph.remove(env_id_);
		graph.remove(trig_id_);
	}

	[[nodiscard]] AudioEndpoint patch_point(ModulePort port) const override {
		auto in_map = [this](auto in) {
			return in == 0 ? audio::make_patch_point(env_id_, 0)
			               : audio::make_patch_point(trig_id_, 0);
		};
		auto out_map = [this](auto) {
			return audio::make_patch_point(env_id_, 0);
		};

		return audio::map_port(port, in_map, out_map);
	}

	float change_control(ControlChange c) override {
		assert(c.index < num_controls());

		switch (c.index) {
		case 0:
			return attack_ctrl_.change(*this, attack_ms_, c.delta);
		case 1:
			return decay_ctrl_.change(*this, decay_ms_, c.delta);
		case 2:
			return sustain_ctrl_.change(*this, sustain_ms_, c.delta);
		case 3:
			return release_ctrl_.change(*this, release_ms_, c.delta);
		}
		return 0;
	}

	[[nodiscard]] const ModuleDisplayInfo &display_info() const override {
		return module_info<Envelope>;
	}

	[[nodiscard]] ColoredText display_text() const override {
		return {"envelope", 0xFFFF00};
	}
	[[nodiscard]] nst::teensy::ColorRGB led_color() const override {
		return 0xFFFF00;
	}

	void trigger(AudioEdge e) {
		switch (e) {
		case AudioEdge::RISING_EDGE:
			env_.noteOn();
			break;
		case AudioEdge::FALLING_EDGE:
			env_.noteOff();
			break;
		}
	}

	void on_rising_edge() override { trigger(AudioEdge::RISING_EDGE); }
	void on_falling_edge() override { trigger(AudioEdge::FALLING_EDGE); }

  public:
	void update_attack() { env_.attack(attack_ms_); }
	void update_decay() { env_.decay(decay_ms_); }
	void update_sustain() { env_.sustain(sustain_ms_); }
	void update_release() { env_.release(release_ms_); }

	struct SustainCoeff {
		constexpr static auto value{0.25f};
	};

	constexpr static auto ms_curve = curves::linear<float, 5>;
	constexpr static auto sustain_curve = curves::linear<float, SustainCoeff>;

	constexpr static auto attack_ctrl_ =
	    ModuleControl{ms_curve, &Self::update_attack};

	constexpr static auto decay_ctrl_ =
	    ModuleControl{ms_curve, &Self::update_decay};

	constexpr static auto sustain_ctrl_ =
	    ModuleControl{sustain_curve, &Self::update_sustain};

	constexpr static auto release_ctrl_ =
	    ModuleControl{ms_curve, &Self::update_release};

	AudioEffectEnvelope env_;
	AudioTriggerInput<Envelope> trig_{*this};
	nst::teensy::AudioNodeID env_id_;
	nst::teensy::AudioNodeID trig_id_;

	using Parameter = ModuleParameter<float>;
	Parameter attack_ms_{0.0f, 500.0f, 10.0f};
	Parameter decay_ms_{0.0f, 500.0f, 35.0f};
	Parameter sustain_ms_{0.0f, 1.0f, 0.0f};
	Parameter release_ms_{0.0f, 500.0f, 100.0f};
};

} // namespace sndbx

#endif