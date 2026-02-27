#ifndef SANDBOX_AUDIO_PASS_THROUGH_HPP_
#define SANDBOX_AUDIO_PASS_THROUGH_HPP_

#include <Arduino.h>
#include <AudioStream.h>

class AudioPassThrough : public AudioStream
{
public:
  AudioPassThrough() : AudioStream(1, m_InputQueueArray) {}
  ~AudioPassThrough() { SAFE_RELEASE_INPUTS(); }

  void update() override
  {
    audio_block_t* block = receiveReadOnly(0);
    if (block) { transmit(block, 0); }
    release(block);
  }

private:
  audio_block_t* m_InputQueueArray[1];
};

#endif