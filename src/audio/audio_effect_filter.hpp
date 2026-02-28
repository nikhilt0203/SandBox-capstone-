#ifndef SANDBOX_AUDIO_FILTER_HPP_
#define SANDBOX_AUDIO_FILTER_HPP_

#include <Audio.h>

class AudioFilter : public AudioFilterStateVariable
{
public:
  enum struct Type
  {
    LOWPASS,
    HIGHPASS,
    BANDPASS
  };

public:
  AudioFilter() = default;
  ~AudioFilter() { SAFE_RELEASE_INPUTS(); };

  void update() override
  {
    audio_block_t *input_block = nullptr, *control_block = nullptr;
    audio_block_t *lowpass_block = nullptr, *bandpass_block = nullptr, *highpass_block = nullptr;

    input_block = receiveReadOnly(0);
    control_block = receiveReadOnly(1);
    if (!input_block) 
    {
      if (control_block) { release(control_block); }
      return;
    }

    lowpass_block = allocate();
    if (!lowpass_block) 
    {
      release(input_block);
      if (control_block) release(control_block);
      return;
    }

    bandpass_block = allocate();
    if (!bandpass_block) 
    {
      release(input_block);
      release(lowpass_block);
      if (control_block) { release(control_block); }
      return;
    }
    
    highpass_block = allocate();
    if (!highpass_block) 
    {
      release(input_block);
      release(lowpass_block);
      release(bandpass_block);
      if (control_block) { release(control_block); }
      return;
    }

    if (control_block) 
    {
      update_variable(input_block->data,
        control_block->data,
        lowpass_block->data,
        bandpass_block->data,
        highpass_block->data);
      release(control_block);
    } 
    else 
    {
      update_fixed(input_block->data,
        lowpass_block->data,
        bandpass_block->data,
        highpass_block->data);
    }

    release(input_block);
    if (m_Current == Type::LOWPASS) { transmit(lowpass_block, 0); }
    release(lowpass_block);
    if (m_Current == Type::BANDPASS) { transmit(bandpass_block, 0); }
    release(bandpass_block);
    if (m_Current == Type::HIGHPASS) { transmit(highpass_block, 0); }
    release(highpass_block);
    return;
  }

  void filterType(int type) noexcept
  { 
    switch (type) 
    {
      case 0: m_Current = Type::LOWPASS; break;
      case 1: m_Current = Type::BANDPASS; break;
      case 2: m_Current = Type::HIGHPASS; break;
      default: break;
    }
  }

private:
  Type m_Current{Type::LOWPASS};
};

#endif