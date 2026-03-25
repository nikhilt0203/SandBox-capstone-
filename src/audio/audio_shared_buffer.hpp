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

    if (s_UpdateBufferResponsibility == m_ID)
    {
      for (std::size_t i{}; i < AUDIO_BLOCK_SAMPLES; ++i)
      {
        if (!isFull())
        {
          const auto sampleNormalized = block->data[i] / 32767.0f;
          s_SharedBuffer[m_BufferIndex] = sampleNormalized;
          m_BufferIndex = m_BufferIndex + 1;
        }
      }
    }

    transmit(block, 0);
    release(block);
  }

  [[nodiscard]] const SampleBuffer& flush() { m_BufferIndex = 0; return s_SharedBuffer; }

  [[nodiscard]] constexpr std::size_t const bufferSize() { return N; }

  [[nodiscard]] bool isFull() const { return m_BufferIndex == N - 1; }

  void setBufferWriteResponsibility() { s_UpdateBufferResponsibility = m_ID; }
  
private:
  audio_block_t* inputQueueArray[1];
  std::size_t m_BufferIndex{};
  std::uint8_t m_ID{s_LastID++};
  inline static std::uint8_t s_LastID{};
  inline static std::uint8_t s_UpdateBufferResponsibility{};
  inline static SampleBuffer s_SharedBuffer{}; //shared among instances
};

#endif