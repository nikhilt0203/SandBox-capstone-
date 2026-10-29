#ifndef SANDBOX_MIXER_HPP_
#define SANDBOX_MIXER_HPP_

#include "controllable.hpp"
#include "displayable.hpp"
#include "module_display_info.hpp"
#include "module_parameter.hpp"
#include "patchable.hpp"

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

class Mixer : public Patchable, public Controllable, public Displayable {
	using Self = Mixer;

  public:
	Mixer()
	    : Patchable{num_ins<Self>, num_outs<Self>},
	      Controllable{num_ctrls<Self>} {}

	AudioError link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	AudioEndpoint patch_point(ModulePort port) const override;

	float change_control(ControlChange c) override;

	const ModuleDisplayInfo &display_info() const override;

  private:
	void update_gains();

	constexpr static auto channels = num_ins<Self>;
	constexpr static auto default_gain = ModuleParameter{0.0f, 1.0f, 0.5f};

	using ChannelGains = std::array<ModuleParameter<float>, channels>;

	template <std::size_t... Is>
	static ChannelGains make_gain_params_impl(std::index_sequence<Is...>) {
		ChannelGains{(((void)Is, default_gain))...};
	}

	static ChannelGains make_gain_params() {
		return make_gain_params_impl(std::make_index_sequence<channels>{});
	}

	struct gain_coeff {
		constexpr static auto value{1.1f};
	};

	AudioMixer4 mix_;
	nst::teensy::AudioNodeID mix_id_;

	ChannelGains channel_gains_{make_gain_params()};

	constexpr static auto gain_ctrl =
	    ModuleControl{curves::linear<float, gain_coeff>, &Self::update_gains};
};

} // namespace sndbx

#endif