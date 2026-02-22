#ifndef lfo_hpp_
#define lfo_hpp_

#include "modules/oscillator.hpp"

class LFO : 
public Oscillator, 
public Pressable
{
public:
  MODULE_TYPE_INFO("lfo", "low frequency oscillator", 0xFFFF00);

public:
  LFO() : Oscillator()
  {
    m_Frequency.value() = 2.0f;
    m_Oscillator->frequency(m_Frequency);
  }

  std::string_view displayName() const override { return m_WaveformNames[m_WaveformIndex]; }

  void onRisingEdge() { m_Oscillator->restart(); }
  void onFallingEdge() {}

private:
  static constexpr std::array<std::string_view, Oscillator::numWaveforms> m_WaveformNames = {
    "sine lfo",
    "square lfo",
    "saw lfo",
    "triangle lfo",
    "pulse lfo",
    "rev saw lfo",
    "s&h noise lfo"
  };
};

#endif