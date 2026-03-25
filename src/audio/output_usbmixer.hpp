#ifndef SANDBOX_OUTPUT_USBMIXER_HPP_
#define SANDBOX_OUTPUT_USBMIXER_HPP_

#include <Audio.h>

class AudioOutputUSBMixer : public AudioOutputUSB
{
public:
  AudioOutputUSBMixer() : AudioOutputUSB() {}
  ~AudioOutputUSBMixer() { SAFE_RELEASE_INPUTS(); }

  void update() override
  {
    audio_block_t *left, *right;

    left = receiveWritable(0); // input 0 = left channel
    right = receiveWritable(1); // input 1 = right channel

    const float scale = m_OutputScale;
    if (left || right)
    {
      for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i)
      {
        if (left) left->data[i] *= scale;
        if (right) right->data[i] *= scale;
      }
    }

    if (usb_audio_transmit_setting == 0) {
      if (left) release(left);
      if (right) release(right);
      if (left_1st) { release(left_1st); left_1st = nullptr; }
      if (left_2nd) { release(left_2nd); left_2nd = nullptr; }
      if (right_1st) { release(right_1st); right_1st = nullptr; }
      if (right_2nd) { release(right_2nd); right_2nd = nullptr; }
      offset_1st = 0;
      return;
    }
    if (left == nullptr) {
      left = allocate();
      if (left == nullptr) {
        if (right) release(right);
        return;
      }
      memset(left->data, 0, sizeof(left->data));
    }
    if (right == nullptr) {
      right = allocate();
      if (right == nullptr) {
        release(left);
        return;
      }
      memset(right->data, 0, sizeof(right->data));
    }
    __disable_irq();
    if (left_1st == nullptr) {
      left_1st = left;
      right_1st = right;
      offset_1st = 0;
    } else if (left_2nd == nullptr) {
      left_2nd = left;
      right_2nd = right;
    } else {
      // buffer overrun - PC is consuming too slowly
      audio_block_t *discard1 = left_1st;
      left_1st = left_2nd;
      left_2nd = left;
      audio_block_t *discard2 = right_1st;
      right_1st = right_2nd;
      right_2nd = right;
      offset_1st = 0; // TODO: discard part of this data?
      //serial_print("*");
      release(discard1);
      release(discard2);
    }
    __enable_irq();
  }

  void volume(float gain) { m_OutputScale = std::clamp(gain, 0.0f, 2.0f); }

private:
  float m_OutputScale{1.0};
};

#endif