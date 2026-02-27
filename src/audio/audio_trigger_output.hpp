#ifndef SANDBOX_AUDIO_TRIGGER_OUTPUT_HPP_
#define SANDBOX_AUDIO_TRIGGER_OUTPUT_HPP_

#include <Arduino.h>
#include <AudioStream.h>
 
class AudioTriggerOutput : public AudioStream
{
public:
  AudioTriggerOutput() : AudioStream(1, m_InputQueueArray) {}

  ~AudioTriggerOutput() { SAFE_RELEASE_INPUTS(); }

  void update() override
  {
    audio_block_t* outBlock = allocate();
    if (!outBlock) { return; }

    if (m_On) 
    {
      for (size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i)
      {
        if (i < 20) outBlock->data[i] = 32767;
      }
      m_On = false;
    }
    else
    {
      for (size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i) 
      {
        outBlock->data[i] = 0;
      }
    }

    transmit(outBlock, 0);
    release(outBlock);
  }

  void on() { m_On = true; }

  void off() { m_On = false; }

private:
  audio_block_t* m_InputQueueArray[1];
  volatile bool m_On{false};
};

#endif