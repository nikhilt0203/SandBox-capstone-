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
    {ctrl_format, ctrl_format, ctrl_format, ctrl_format}};

class Mixer : public Patchable, public Controllable, public Displayable {
	constexpr static auto num_channels = 4U;

  public:
	Mixer() : Patchable{num_channels, 1}, Controllable{4} {}

	bool link(AudioGraph &graph) override;
	void unlink(AudioGraph &graph) override;
	AudioPoint patch_point(ModulePort port) const override;

	float change_control(ControlChange c) override;

	const ModuleDisplayInfo &display_info() const override;

  private:
	void update_gains();

	constexpr static auto default_gain = ModuleParameter{0.0f, 1.0f, 0.5f};

	using ChannelGains = std::array<ModuleParameter<float>, num_channels>;

	template <std::size_t... Is>
	static ChannelGains make_gain_params(std::index_sequence<Is...>) {
		return ChannelGains{(((void)Is, default_gain))...};
	}

	AudioMixer4 mix_;
	nst::teensy::AudioNodeID mix_id_;

	ChannelGains channel_gains_{
	    make_gain_params(std::make_index_sequence<num_channels>{})};

	constexpr static auto gain_ctrl =
	    ModuleControl{curves::default_gain_curve, &Mixer::update_gains};
};

} // namespace sndbx

#endif