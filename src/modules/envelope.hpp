#ifndef SANDBOX_ENVELOPE_HPP_
#define SANDBOX_ENVELOPE_HPP_

#include "audio/audio_engine.hpp"
#include "audio/audio_trigger.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "parameter.hpp"
#include "pressable.hpp"

namespace sndbx {

class Envelope;

template <>
inline constexpr ModuleDisplayInfo module_info<Envelope>{
    {"envelope", 0xFFFF00},
    "applies an envelope to the input signal",
    {"in", "trg"},
    {"out"},
    {"attack", "decay", "sustain", "release"}};

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
		graph.remove_node(env_id_);
		graph.remove_node(trig_id_);
	}

	[[nodiscard]] AudioEndpoint endpoint(ModulePort port) const override {
		auto in_map = [this](auto in) {
			return in == 0 ? audio::make_endpoint(env_id_)
			               : audio::make_endpoint(trig_id_);
		};
		auto out_map = [this](auto) { return audio::make_endpoint(env_id_); };

		return audio::map_port(port, in_map, out_map);
	}

	float change_control(std::uint8_t idx, std::int8_t amt) override {
		switch (idx) {
		case 0:
			change_attack(amt);
			return attack_ms_.ratio();
		case 1:
			change_decay(amt);
			return decay_ms_.ratio();
		case 2:
			change_sustain(amt);
			return sustain_ms_.ratio();
		case 3:
			change_release(amt);
			return release_ms_.ratio();
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

  protected:
	void change_attack(int ms) { env_.attack(attack_ms_ += ms); }
	void change_decay(int ms) { env_.decay(decay_ms_ += ms); }
	void change_sustain(int ms) { env_.sustain(decay_ms_ += ms); }
	void change_release(int ms) { env_.release(decay_ms_ += ms); }

	ModuleParameter<float> attack_ms_{0.0f, 500.0f, 10.0f};
	ModuleParameter<float> decay_ms_{0.0f, 500.0f, 35.0f};
	ModuleParameter<float> sustain_ms_{0.0f, 1.0f, 0.0f};
	ModuleParameter<float> release_ms_{0.0f, 500.0f, 100.0f};

	AudioEffectEnvelope env_;
	nst::teensy::AudioNodeID env_id_;
	AudioTriggerInput<Envelope> trig_{*this};
	nst::teensy::AudioNodeID trig_id_;
};

} // namespace sndbx

#endif