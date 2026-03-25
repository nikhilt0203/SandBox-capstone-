#ifndef SANDBOX_AUDIO_TRIGGER_INPUT_HPP_
#define SANDBOX_AUDIO_TRIGGER_INPUT_HPP_

#include <Arduino.h>
#include <AudioStream.h>

class AudioTriggerInput : public AudioStream
{
public:
  AudioTriggerInput() : AudioStream(1, m_InputQueueArray) {}
  ~AudioTriggerInput() { SAFE_RELEASE_INPUTS(); }

  void threshold(float value) noexcept { m_Threshold = value; }

  void risingEdgeCallback(std::function<void()> f) { m_RisingEdgeCallback = f; }
  void fallingEdgeCallback(std::function<void()> f) { m_FallingEdgeCallback = f; }

  void update() override
  {
    audio_block_t* block = receiveReadOnly(0);

    if (!block) 
    {
      if (m_RisingEdgeTriggeredLast) 
      {
        m_RisingEdgeTriggeredLast = false;
        if (m_FallingEdgeCallback) { m_FallingEdgeCallback(); }
      }
      return;
    }

    bool sawHigh = false;

    for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
      const float sample = block->data[i] / 32767.0f;
      if (sample >= m_Threshold) 
      { 
        sawHigh = true; 
        break; 
      }
    }

    if (sawHigh && !m_RisingEdgeTriggeredLast) 
    {
      m_RisingEdgeTriggeredLast = true;
      if (m_RisingEdgeCallback) { m_RisingEdgeCallback(); }
    } 
    else if (!sawHigh && m_RisingEdgeTriggeredLast)
    {
      m_RisingEdgeTriggeredLast = false;
      if (m_FallingEdgeCallback) { m_FallingEdgeCallback(); }
    }

    release(block);
  }

private:
  audio_block_t* m_InputQueueArray[1];
  float m_Threshold{0.25f};

  std::function<void()> m_RisingEdgeCallback{};
  std::function<void()> m_FallingEdgeCallback{};

  bool m_FallingEdgeTriggeredLast{false};
  bool m_RisingEdgeTriggeredLast{false};
};

#endif