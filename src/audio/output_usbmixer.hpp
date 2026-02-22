#ifndef output_usbmixer_hpp_
#define output_usbmixer_hpp_

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
      for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; i++)
      {
        if (left) left->data[i] *= scale;
        if (right) right->data[i] *= scale;
      }
    }

    if (usb_audio_transmit_setting == 0) {
      if (left) release(left);
      if (right) release(right);
      if (left_1st) { release(left_1st); left_1st = NULL; }
      if (left_2nd) { release(left_2nd); left_2nd = NULL; }
      if (right_1st) { release(right_1st); right_1st = NULL; }
      if (right_2nd) { release(right_2nd); right_2nd = NULL; }
      offset_1st = 0;
      return;
    }
    if (left == NULL) {
      left = allocate();
      if (left == NULL) {
        if (right) release(right);
        return;
      }
      memset(left->data, 0, sizeof(left->data));
    }
    if (right == NULL) {
      right = allocate();
      if (right == NULL) {
        release(left);
        return;
      }
      memset(right->data, 0, sizeof(right->data));
    }
    __disable_irq();
    if (left_1st == NULL) {
      left_1st = left;
      right_1st = right;
      offset_1st = 0;
    } else if (left_2nd == NULL) {
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