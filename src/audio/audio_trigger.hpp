#ifndef SANDBOX_AUDIO_TRIGGER_INPUT_HPP_
#define SANDBOX_AUDIO_TRIGGER_INPUT_HPP_

#include <Arduino.h>
#include <AudioStream.h>

namespace sndbx {

enum class AudioEdge { RISING_EDGE, FALLING_EDGE };

// 'Triggerable' models an object where 'trigger(sndbx::AudioEdge)' is a
// valid public method
template <class Triggerable> class AudioTriggerInput : public AudioStream {
public:
  AudioTriggerInput(Triggerable &t)
      : AudioStream(1, input_array_), target_{t} {}

  ~AudioTriggerInput() { SAFE_RELEASE_INPUTS(); }

  void set_threshold(float level) { threshold_ = level; }

  void update() override {
    audio_block_t *block = receiveReadOnly(0);

    if (!block) {
      if (last_rising_) {
        last_rising_ = false;
        target_.trigger(AudioEdge::FALLING_EDGE);
      }
      return;
    }

    bool saw_high;

    for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i) {
      const float sample = block->data[i] / 32767.0f;
      if (sample >= threshold_) {
        saw_high = true;
        break;
      }
    }

    if (saw_high && !last_rising_) {
      last_rising_ = true;
      target_.trigger(AudioEdge::RISING_EDGE);
    } else if (!saw_high && last_rising_) {
      last_rising_ = false;
      target_.trigger(AudioEdge::FALLING_EDGE);
    }

    release(block);
  }

private:
  audio_block_t *input_array_[1];
  Triggerable &target_;
  float threshold_{0.25f};
  bool last_rising_{};
};

} // namespace sndbx

#endif