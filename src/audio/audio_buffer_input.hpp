#ifndef SANDBOX_AUDIO_BUFFER_INPUT_HPP_
#define SANDBOX_AUDIO_BUFFER_INPUT_HPP_

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

    for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i)
    {
      float sampleNormalized = block->data[i] / 32767.0f;

      // ring buffer write
      m_SampleBuffer[m_BufferIndex] = sampleNormalized;
      m_BufferIndex = (m_BufferIndex + 1) % bufferSize();
    }

    transmit(block, 0);
    release(block);
  }

  const SampleBuffer& flush() { return m_SampleBuffer; }

  constexpr std::size_t const bufferSize() { return N; }

  bool available() const { return m_BufferIndex == bufferSize() - 1; }
  
private:
  audio_block_t* inputQueueArray[1];
  volatile std::size_t m_BufferIndex{};
  inline static SampleBuffer m_SampleBuffer{}; //shared among instances
};

#endif