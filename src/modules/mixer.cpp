#pragma once
#include "dep/module.hpp"
#include "dep/controls.hpp"
#include "dep/module_interfaces.hpp"
#include "mixer.hpp"

Mixer::Mixer() : Module(4, 1)
{
  m_Audio.addDevice<AudioMixer4>();
  m_Mixer = m_Audio.device<AudioMixer4>();

  m_Audio.mapOutput(0, m_Mixer, 0);
  for (std::size_t i{}; i < numInputs(); i++) { m_Audio.mapInput(i, m_Mixer, i); }
}

const std::vector<float>& Mixer::normalizedControlValues() const
{
  m_NormalizedControlValues.reserve(4);
  m_NormalizedControlValues.clear();
  for (auto gain : m_ChannelGains) { m_NormalizedControlValues.push_back(gain.normalized()); }
  return m_NormalizedControlValues;
}

void Mixer::gainAdjust(std::size_t channel, int delta) 
{ 
  auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1*d); };
  auto& gain = m_ChannelGains.at(channel);
  gain.change(gainCurve, delta); 
  m_Mixer->gain(channel, gain);
}