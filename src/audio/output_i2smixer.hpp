#ifndef output_i2smixer_hpp_
#define output_i2smixer_hpp_

#include <Audio.h>

class AudioOutputI2SMixer : public AudioOutputI2S
{
public:
  AudioOutputI2SMixer() = default;
  ~AudioOutputI2SMixer() { SAFE_RELEASE_INPUTS(); }

  void update() override
  {
    auto left = receiveReadOnly(0); // input 0 = left channel
    auto right = receiveReadOnly(1);
    const float scale = m_OutputScale;
    if (left || right)
    {
      for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; i++)
      {
        if (left) left->data[i] *= scale;
        if (right) right->data[i] *= scale;
      }
    }

    if (left) {
      __disable_irq();
      if (block_left_1st == nullptr) {
        block_left_1st = left;
        block_left_offset = 0;
        __enable_irq();
      } else if (block_left_2nd == nullptr) {
        block_left_2nd = left;
        __enable_irq();
      } else {
        audio_block_t *tmp = block_left_1st;
        block_left_1st = block_left_2nd;
        block_left_2nd = left;
        block_left_offset = 0;
        __enable_irq();
        release(tmp);
      }
    }

    if (right) {
      __disable_irq();
      if (block_right_1st == nullptr) {
        block_right_1st = right;
        block_right_offset = 0;
        __enable_irq();
      } else if (block_right_2nd == nullptr) {
        block_right_2nd = right;
        __enable_irq();
      } else {
        audio_block_t *tmp = block_right_1st;
        block_right_1st = block_right_2nd;
        block_right_2nd = right;
        block_right_offset = 0;
        __enable_irq();
        release(tmp);
      }
    }
  }

  void volume(float gain) { m_OutputScale = std::clamp(gain, 0.0f, 2.0f); }

private:
  float m_OutputScale{0.5f};
};

#endif