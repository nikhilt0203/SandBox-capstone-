#ifndef SANDBOX_AUDIO_OUT_HPP_
#define SANDBOX_AUDIO_OUT_HPP_

#include "controllable.hpp"
#include "displayable.hpp"
#include "io/serializable.hpp"
#include "patchable.hpp"

namespace sndbx {
class AudioOut;

template <>
inline constexpr ModuleDisplayInfo module_info<AudioOut>{
    {"audio out", 0x0000FF}, "sends to the audio output", {"gain"}, {"in"}, {}};

class AudioOut : public Patchable,
                 public Controllable,
                 public Displayable,
                 public Serializable {
  public:
	AudioOut() : Patchable{1, 0}, Controllable{1} { update_gain(); }

	bool link(AudioGraph &graph) override;
	AudioPoint patch_point(ModulePort port) const override;
	void unlink(AudioGraph &graph) override;

	float change_control(ControlChange cc) override;

	const ModuleDisplayInfo &display_info() const override;

	void serialize(SerializationBuffer &sb) const override {
		save_params(sb, gain_);
	}

	void deserialize(SerializationBuffer &sb) override {
		load_params(sb, gain_);
	}

  private:
	void update_gain();

	AudioAmplifier amp_;
	AudioOutputUSB usb_out_;
	inline static AudioOutputI2S i2s_out_; // needs to be static to get audio

	nst::teensy::AudioID usb_id_;
	nst::teensy::AudioID i2s_id_;
	nst::teensy::AudioID amp_id_;

	ModuleParameter<float> gain_{0.0f, 1.0f, 0.5f};

	constexpr static ModuleControl gain_ctrl{curves::gain_curve,
	                                         &AudioOut::update_gain};
};

} // namespace sndbx
#endif