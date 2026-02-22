#include "modules/oscillator.hpp"

std::array<Oscillator::Waveform, Oscillator::numWaveforms> Oscillator::m_Waveforms = {
  Waveform{ WAVEFORM_SINE,             "sine",      0x00FF00 },
  Waveform{ WAVEFORM_SQUARE,           "square",    0xFF0000 },
  Waveform{ WAVEFORM_SAWTOOTH,         "saw",       0xFF00FF },
  Waveform{ WAVEFORM_TRIANGLE,         "triangle",  0xFFFF00 },
  Waveform{ WAVEFORM_PULSE,            "pulse",     0x0083C2 },
  Waveform{ WAVEFORM_SAWTOOTH_REVERSE, "rev saw",   0xB8127E },
  Waveform{ WAVEFORM_SAMPLE_HOLD,      "s&h noise", 0x09F877 }
};

Oscillator::Oscillator() : Module(2, 1)
{
  m_Audio.addDevice<AudioSynthWaveformModulated>();
  m_Oscillator = m_Audio.device<AudioSynthWaveformModulated>();

  m_Oscillator->begin(0.5, m_Frequency, m_Waveforms.at(m_WaveformIndex).id);
  m_Oscillator->frequencyModulation(8.0f);

  m_Audio.mapInput(0, m_Oscillator, 0);
  m_Audio.mapInput(1, m_Oscillator, 1);
  m_Audio.mapOutput(0, m_Oscillator, 0);
}

void Oscillator::frequencyAdjustCoarse(int delta) 
{
  static auto coarseCurve = [](float cur, int delta){ return cur * powf(1.08f, 1*delta); };
  m_Frequency.change(coarseCurve, delta);
  m_Oscillator->frequency(m_Frequency + m_FineTuneOffset);
}

void Oscillator::frequencyAdjustFine(int delta) 
{
  static auto fineCurve = [](int cur, int delta){ return cur + 1*delta; };
  m_FineTuneOffset.change(fineCurve, delta);
  m_Oscillator->frequency(m_Frequency + m_FineTuneOffset);
}

void Oscillator::fmDepthAdjust(int delta) 
{
  static auto fmCurve = [](float cur, int delta){ return cur + 0.25f*delta; };
  m_FMDepth.change(fmCurve, delta);
  m_Oscillator->frequencyModulation(m_FMDepth);
}

void Oscillator::waveformAdjust(int delta) 
{
  static auto waveCurve = [](std::size_t cur, int delta){ 
    auto index = static_cast<int>(cur) + delta;
    return static_cast<std::size_t>(index % static_cast<int>(numWaveforms));
  };

  m_WaveformIndex.change(waveCurve, delta);
  const auto newWaveform = m_Waveforms.at(m_WaveformIndex).id;
  m_Oscillator->begin(newWaveform);
}