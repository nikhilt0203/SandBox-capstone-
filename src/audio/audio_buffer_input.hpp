#ifndef audio_buffer_input_hpp_
#define audio_buffer_input_hpp_

#include <Audio.h>
#include <array>

template<std::size_t N>
class AudioBufferInput: public AudioStream
{
public:
  using SampleBuffer = std::array<float, N>;
public:
  AudioBufferInput() : AudioStream(1, inputQueueArray) {}
  ~AudioBufferInput() { SAFE_RELEASE_INPUTS(); }

  void update() override
  {
    audio_block_t* block = receiveReadOnly(0);
    if (!block) { return; }

    for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; i++)
    {
      std::size_t nextIndex = m_BufferIndex + 1;
      
      if (nextIndex >= bufferSize()) { break; }

      float sampleNormalized = block->data[i] / 32767.0f;
      m_SampleBuffer[nextIndex % bufferSize()] = sampleNormalized;
      m_BufferIndex = nextIndex;
    }

    if (block) { transmit(block, 0); }
    release(block);
  }

  const SampleBuffer& flush() 
  {
    m_BufferIndex = 0;
    return m_SampleBuffer;
  }

  constexpr std::size_t const bufferSize() { return N; }

  bool available() const { return m_BufferIndex == bufferSize() - 1; }
  
private:
  audio_block_t* inputQueueArray[1];
  volatile std::size_t m_BufferIndex{};
  SampleBuffer m_SampleBuffer{};
};

#endif