#ifndef SANDBOX_MIXER_HPP_
#define SANDBOX_MIXER_HPP_

#include "audio/audio_engine.hpp"
#include "audio/audio_trigger.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"
#include "pressable.hpp"

namespace sndbx {
class Mixer;

template <>
inline constexpr ModuleDisplayInfo module_info<Mixer>{
    {"mixer", 0x88FF09},
    "4 channel mixer",
    {"gain 1", "gain 2", "gain 3", "gain 4"},
    {"1", "2", "3", "4"},
    {"out"},
    {ctrl_format<FMillis>, ctrl_format<FMillis>, ctrl_format<FMillis>,
     ctrl_format<FMillis>}};

class Mixer : public audio::Patchable, public Controllable, public Displayable {
	using Self = Mixer;

  public:
	Mixer()
	    : Patchable{num_ins<Self>, num_outs<Self>},
	      Controllable{num_ctrls<Self>} {
		channel_gains_.fill(default_gain);
	}

	AudioError link(AudioGraph &graph) override {
		return audio::add_nodes(graph, {&mix_, mix_id_});
	}

	void unlink(AudioGraph &graph) override { graph.remove(mix_id_); }

	AudioEndpoint patch_point(ModulePort port) const override {
		using namespace audio;
		return map_port(
		    port,
		    [this](auto in) {
			    return in == 0 ? make_endpoint(env_id_, 0)
			                   : make_endpoint(trig_id_, 0);
		    },
		    [this](auto out) { return make_endpoint(env_id_, 0); });
	}

	float change_control(ControlChange c) override {
		assert(c.index < num_controls());
		return gain_ctrl.change(*this, channel_gains_[c.index], c.delta);
	}

	[[nodiscard]] const ModuleDisplayInfo &display_info() const override {
		return module_info<Mixer>;
	}

  public:
	void update_gains() {
		for (std::size_t ch{}; ch < channel_gains_.max_size(); ++ch) {
			mix_.gain(ch, channel_gains_[ch]);
		}
	}

	AudioMixer4 mix_;
	nst::teensy::AudioNodeID mix_id_;

	constexpr static auto default_gain = ModuleParameter{0.0f, 1.0f, 0.5f};
	std::array<ModuleParameter<float>, num_ins<Self>> channel_gains_;

	struct gain_coeff {
		constexpr static auto value{1.1f};
	};

	constexpr static auto gain_ctrl =
	    ModuleControl{curves::linear<float, gain_coeff>, &Self::update_gains};
};

} // namespace sndbx

#endif