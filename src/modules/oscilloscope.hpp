#ifndef SANDBOX_AUDIO_OUT_HPP_
#define SANDBOX_AUDIO_OUT_HPP_

#include "animatable.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "io/serializable.hpp"
#include "patchable.hpp"
#include "ui/screen_elements.hpp"

namespace sndbx {

template <std::size_t N> class AudioBuffer : public AudioStream {
  public:
	using SampleBuffer = std::array<float, N>;

  public:
	AudioBuffer() : AudioStream(1, input_) {}
	~AudioBuffer() { SAFE_RELEASE_INPUTS(); }

	void update() override {
		audio_block_t *block = receiveReadOnly(0);
		if (!block) {
			return;
		}

		for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i) {
			if (index < buffer_.) {
				buffer_[index_++] = block->data[i] / 32767.0f;
			}
		}

		transmit(block, 0);
		release(block);
	}

	[[nodiscard]] const auto flush() {
		index_ = 0;
		return buffer_;
	}

  private:
	audio_block_t *input_[1];
	std::size_t index_{};
	SampleBuffer buffer_{};
};

class Oscilloscope;

template <>
inline constexpr ModuleDisplayInfo module_info<Oscilloscope>{
    {"scope", 0x00FFFF}, "displays the input signal", {}, {}, {}};

class Oscilloscope : public Patchable, public Displayable, public Animatable {
  public:
	Oscilloscope() : Patchable{1, 1} {}

	bool link(AudioGraph &graph) override {
		return audio::register_nodes(graph, {&buffer_, buffer_id_});
	}

	AudioPoint patch_point(ModulePort port) const override {
		assert(port.index == 0);
		return audio::make_audio_point(buffer_id_);
	}

	void unlink(AudioGraph &graph) override { graph.remove(buffer_id_); }

	const ModuleDisplayInfo &display_info() const override {
		return module_info<Oscilloscope>;
	}

	void draw(GFXcanvas16 &frame) const override {
		ui::OscilloscopeFrame d{buffer_.flush(), frame};
	}

  private:
	mutable AudioBuffer<1024> buffer_;
	nst::teensy::AudioID buffer_id_;
};

} // namespace sndbx
#endif