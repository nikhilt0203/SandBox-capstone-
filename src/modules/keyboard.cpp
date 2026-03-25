#include "keyboard.hpp"
#include "ui/color.hpp"

Keyboard::Keyboard() : Module(0, 2) 
{
  m_Audio.addDevice<AudioSynthWaveformDc>();
  m_Audio.addDevice<AudioTriggerOutput>();

  m_DC = m_Audio.device<AudioSynthWaveformDc>();
  m_TrigOut = m_Audio.device<AudioTriggerOutput>(1);
  m_Audio.mapOutput(0, m_DC, 0);
  m_Audio.mapOutput(1, m_TrigOut, 0);
}

void Keyboard::changeControl(std::size_t index, int delta)
{
  switch (index)
  {
    case 0: lengthAdjust(delta); break;
    case 1: scaleAdjust(delta); break;
    default: return;
  }
}

void Keyboard::resetControls()
{
  m_NumKeys.reset();
  m_Scale.reset();
}

auto Keyboard::normalizedControlValues() const -> const sndbx::vector_4U<float>&
{
  m_ControlValues.clear();
  m_ControlValues.push_back(m_NumKeys.normalized());
  m_ControlValues.push_back(m_Scale.normalized());
  return m_ControlValues;
}

void Keyboard::onKeyPress(float amplitude)
{
  m_DC->amplitude(amplitude);
  ++m_NumKeysOn;
  m_TrigOut->on();
}

void Keyboard::onKeyRelease()
{
  --m_NumKeysOn;
  if (m_NumKeysOn == 0) { m_TrigOut->off(); }
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

Keyboard::Scale getScale(std::uint8_t index)
{
  switch (index)
  {
    case 0: return Keyboard::Scale::Major;
    case 1: return Keyboard::Scale::Minor;
    case 2: return Keyboard::Scale::MajorPentatonic;
    case 3: return Keyboard::Scale::MinorPentatonic;
    default: return Keyboard::Scale::Major;
  }
}

Keyboard::Scale Keyboard::scale() { return getScale(m_Scale); }

void Keyboard::scaleAdjust(int delta)
{
  auto scaleCurve = [](std::uint8_t cur, int delta) 
    -> std::uint8_t { return cur + 1*delta; };

  if (!m_ScaleChangeCallback) { return; }

  m_Scale.change(scaleCurve, delta);
  m_ScaleChangeCallback(getScale(m_Scale), id());
}

auto KeyboardKey::normalizedControlValues() const -> const sndbx::vector_4U<float>&
{
  m_ControlValues.clear();
  m_ControlValues.push_back(m_Amplitude);
  return m_ControlValues;
}

void KeyboardKey::updateColor()
{
  m_LEDColor = sndbx::color::blend(COLOR, 0xDD0FFF, m_Amplitude);
}

void KeyboardKey::adjustAmplitude(int delta)
{
  auto amplitudeCurve = [](float cur, int delta){ return cur + 0.05f*delta; };
  m_Amplitude.change(amplitudeCurve, delta);
  updateColor();
}

void KeyboardKey::setAmplitude(float amplitude) 
{ 
  m_Amplitude.value() = std::clamp(amplitude, m_Amplitude.min(), m_Amplitude.max()); 
  updateColor();
}
