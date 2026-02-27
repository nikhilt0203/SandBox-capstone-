#include "keyboard.hpp"

Keyboard::Keyboard() : Module(0, 2) 
{
  m_Audio.addDevice<AudioSynthWaveformDc>();
  m_Audio.addDevice<AudioTriggerOutput>();

  m_DC = m_Audio.device<AudioSynthWaveformDc>();
  m_TrigOut = m_Audio.device<AudioTriggerOutput>(1);
  m_Audio.mapOutput(0, m_DC, 0);
  m_Audio.mapOutput(1, m_TrigOut, 0);
}

auto Keyboard::normalizedControlValues() const -> const std::vector<float>&
{
  m_ControlValues.clear();
  m_ControlValues.push_back(m_NumKeys);
  return m_ControlValues;
}

void Keyboard::on(float amplitude)
{
  m_DC->amplitude(amplitude);
  m_TrigOut->on();
}

void Keyboard::lengthAdjust(int delta)
{
  auto lenCurve = [](std::size_t cur, int delta){ return cur + 1*delta; };

  if (!m_AddKeyCallback || !m_SubtractKeyCallback) { return; }

  bool success = false;
  if (delta > 0 && m_NumKeys < m_NumKeys.max()) { success = m_AddKeyCallback(*this); }
  if (delta < 0 && m_NumKeys > m_NumKeys.min()) { success = m_SubtractKeyCallback(*this); }

  if (success) { m_NumKeys.change(lenCurve, delta); }
}

auto KeyboardKey::normalizedControlValues() const -> const std::vector<float>&
{
  m_ControlValues.clear();
  m_ControlValues.push_back(m_Amplitude);
  return m_ControlValues;
}

void KeyboardKey::adjustAmplitude(int delta)
{
  auto amplitudeCurve = [](float cur, int delta){ return cur + 0.05f*delta; };
  m_Amplitude.change(amplitudeCurve, delta);
}
