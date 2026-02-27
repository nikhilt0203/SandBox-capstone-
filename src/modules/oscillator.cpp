#include "modules/oscillator.hpp"

const std::array<Oscillator::Waveform, Oscillator::numWaveforms> Oscillator::m_Waveforms = {
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

  m_Oscillator->begin(0.5f, m_Frequency, m_Waveforms.at(m_WaveformIndex).id);
  m_Oscillator->frequencyModulation(m_FMDepth);

  m_Audio.mapInput(0, m_Oscillator, 0);
  m_Audio.mapInput(1, m_Oscillator, 1);
  m_Audio.mapOutput(0, m_Oscillator, 0);
}

Oscillator::Oscillator(float frequency, int fineTuneOffset, float fmDepth, std::size_t waveform)
  : Oscillator()
{
  m_Frequency.setValue(frequency);
  m_FineTuneOffset.setValue(fineTuneOffset);
  m_FMDepth.setValue(fmDepth);
  m_WaveformIndex.setValue(waveform);

  m_Oscillator->begin(0.5f, m_Frequency + m_FineTuneOffset, m_Waveforms.at(m_WaveformIndex).id);
  m_Oscillator->frequencyModulation(m_FMDepth);
}

auto Oscillator::normalizedControlValues() const -> const std::vector<float>&
{ 
  m_ControlValues.reserve(4); 
  m_ControlValues.clear();
  m_ControlValues.push_back(std::log((std::log(m_Frequency) / std::log(1.08f))));
  m_ControlValues.push_back(std::log(m_FineTuneOffset.normalized()));
  m_ControlValues.push_back(std::log(m_FMDepth.normalized()));
  m_ControlValues.push_back(std::log(m_WaveformIndex.normalized()));
  return m_ControlValues;
}

void Oscillator::frequencyAdjustCoarse(int delta) 
{
  auto coarseCurve = [](float cur, int delta){ return cur * powf(1.08f, 1*delta); };
  m_Frequency.change(coarseCurve, delta);
  m_Oscillator->frequency(m_Frequency + m_FineTuneOffset);
}

void Oscillator::frequencyAdjustFine(int delta) 
{
  auto fineCurve = [](int cur, int delta){ return cur + 1*delta; };
  m_FineTuneOffset.change(fineCurve, delta);
  m_Oscillator->frequency(m_Frequency + m_FineTuneOffset);
}

void Oscillator::fmDepthAdjust(int delta) 
{
  auto fmCurve = [](float cur, int delta){ return cur + 0.25f*delta; };
  m_FMDepth.change(fmCurve, delta);
  m_Oscillator->frequencyModulation(m_FMDepth);
}

void Oscillator::waveformAdjust(int delta) 
{
  auto waveCurve = [](std::size_t cur, int delta){ 
    auto index = static_cast<int>(cur) + delta;
    return static_cast<std::size_t>(index % static_cast<int>(numWaveforms));
  };

  m_WaveformIndex.change(waveCurve, delta);
  const auto newWaveform = m_Waveforms.at(m_WaveformIndex).id;
  m_Oscillator->begin(newWaveform);
}