#ifndef audio_trigger_input_hpp_
#define audio_trigger_input_hpp_

#include <Arduino.h>
#include <AudioStream.h>

class AudioTriggerInput : public AudioStream
{
public:
  AudioTriggerInput() : AudioStream(1, m_InputQueueArray) {}
  ~AudioTriggerInput() { SAFE_RELEASE_INPUTS(); }

  void threshold(float value) { m_Threshold = value; }

  void risingEdgeCallback(std::function<void()> f) { m_RisingEdgeCallback = f; }
  void fallingEdgeCallback(std::function<void()> f) { m_FallingEdgeCallback = f; }

  void update() override
  {
    audio_block_t* block = receiveReadOnly(0);

    if (!block) 
    {
      m_RisingEdgeTriggeredLast = false;
      m_FallingEdgeTriggeredLast = false;
      return;
    }

    for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; i++)
    {
      const float sample = block->data[i] / 32767.0f;

      if (sample >= m_Threshold && !m_RisingEdgeTriggeredLast)
      {
        m_RisingEdgeTriggeredLast = true;
        m_FallingEdgeTriggeredLast = false;

        if (m_RisingEdgeCallback) { m_RisingEdgeCallback(); }
      }

      if (sample < m_Threshold && !m_FallingEdgeTriggeredLast)
      {
        m_FallingEdgeTriggeredLast = true;
        m_RisingEdgeTriggeredLast = false;

        if (m_FallingEdgeCallback) { m_FallingEdgeCallback(); }
      }
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