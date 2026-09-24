#ifndef SANDBOX_LFO_HPP_
#define SANDBOX_LFO_HPP_

#include "modules/oscillator.hpp"

class LFO final : public Oscillator, public Pressable {
public:
  MODULE_TYPE_INFO("lfo", "low frequency oscillator", 0xFFFF00);
  constexpr static auto max_count = 32U;

public:
  LFO() : Oscillator() { resetControls(); }

  LFO(float frequency, int fineTuneOffset, float fmDepth, std::size_t waveform)
      : Oscillator(frequency, fineTuneOffset, fmDepth, waveform) {}

  void resetControls() override {
    m_Frequency.value() = 2.0f;
    m_Oscillator->frequency(m_Frequency);
  }

  std::string_view displayName() const override {
    return m_WaveformNames[m_WaveformIndex];
  }

  void onRisingEdge() { m_Oscillator->restart(); }
  void onFallingEdge() {}

private:
  static inline constexpr std::array<std::string_view, Oscillator::numWaveforms>
      m_WaveformNames{"sine lfo",     "square lfo", "saw lfo",
                      "triangle lfo", "pulse lfo",  "rev saw lfo",
                      "s&h noise lfo"};
};

#endif