#ifndef SANDBOX_AUDIO_OUT_HPP_
#define SANDBOX_AUDIO_OUT_HPP_

#include "controllable.hpp"
#include "displayable.hpp"
#include "patchable.hpp"

namespace sndbx {
class AudioOut;

template <>
inline constexpr ModuleDisplayInfo module_info<AudioOut>{
    {"audio out", 0x0000FF},
    "sends to the audio output",
    {"gain"},
    {"in"},
    {},
    {}};

class AudioOut : public Patchable, public Controllable, public Displayable {
  public:
	AudioOut() : Patchable{1, 0}, Controllable{1} {}

	bool link(AudioGraph &graph) override;
	AudioPoint patch_point(ModulePort port) const override;
	void unlink(AudioGraph &graph) override;

	float change_control(ControlChange cc) override;

	const ModuleDisplayInfo &display_info() const override;

  private:
	void update_gain();

	AudioAmplifier amp_;
	AudioOutputUSB usb_out_;
	AudioOutputI2S i2s_out_;

	nst::teensy::AudioNodeID usb_id_;
	nst::teensy::AudioNodeID i2s_id_;
	nst::teensy::AudioNodeID amp_id_;

	ModuleParameter<float> gain_{0.0f, 1.0f, 0.5f};

	constexpr static ModuleControl gain_ctrl{curves::gain_curve,
	                                         &AudioOut::update_gain};
};

} // namespace sndbx
#endif