#include "mixer.hpp"

Mixer::Mixer() : Module(4, 1)
{
  m_Audio.addDevice<AudioMixer4>();
  m_Mixer = m_Audio.device<AudioMixer4>();
  setGains();

  m_Audio.mapOutput(0, m_Mixer, 0);

  for (std::size_t i{}; i < numInputs(); ++i) { m_Audio.mapInput(i, m_Mixer, i); }
}

Mixer::Mixer(std::initializer_list<float> gains) : Mixer()
{
  std::size_t i{};
  for (const auto gain : gains)
  {
    if (i < numChannels) { m_ChannelGains.at(i).setValue(gain); }
    ++i;
  }
  setGains();
}

void Mixer::setGains()
{
  for (std::size_t i{}; i < numChannels; ++i)
  {
    m_Mixer->gain(i, m_ChannelGains[i]);
  }
}

void Mixer::resetControls()
{
  for (auto& gain : m_ChannelGains) { gain.reset(); }
  setGains();
}

auto Mixer::normalizedControlValues() const -> const sndbx::vector_4U<float>& 
{
  m_NormalizedControlValues.clear();

  for (auto gain : m_ChannelGains) { m_NormalizedControlValues.push_back(gain.normalized()); }

  return m_NormalizedControlValues;
}

void Mixer::gainAdjust(std::size_t channel, int delta) 
{
  auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1 * d); };
  auto& gain = m_ChannelGains.at(channel);
  gain.change(gainCurve, delta);
  m_Mixer->gain(channel, gain);
}