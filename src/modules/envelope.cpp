#include "envelope.hpp"

Envelope::Envelope() : Module(2, 2)
{
  m_Audio.addDevice<AudioEffectEnvelope>();
  m_Audio.addDevice<AudioTriggerInput>();
  m_Audio.addDevice<AudioTriggerOutput>();

  m_Envelope = m_Audio.device<AudioEffectEnvelope>();
  auto trigIn = m_Audio.device<AudioTriggerInput>(1);
  m_TrigOut = m_Audio.device<AudioTriggerOutput>(2);

  m_Envelope->attack(m_Attack);
  m_Envelope->decay(m_Decay);
  m_Envelope->sustain(m_Sustain);
  m_Envelope->release(m_Release);

  trigIn->risingEdgeCallback([this](){ onRisingEdge(); });
  trigIn->fallingEdgeCallback([this](){ onFallingEdge(); });

  m_Audio.mapInput(0, m_Envelope, 0);
  m_Audio.mapInput(1, trigIn, 0);
  m_Audio.mapOutput(0, m_Envelope, 0);
  m_Audio.mapOutput(1, m_TrigOut, 0);
}

auto Envelope::normalizedControlValues() const -> const sndbx::vector_4U<float>&
{ 
  m_ControlValues.clear();
  m_ControlValues.push_back(m_Attack.normalized());
  m_ControlValues.push_back(m_Decay.normalized());
  m_ControlValues.push_back(m_Sustain.normalized());
  m_ControlValues.push_back(m_Release.normalized());
  return m_ControlValues;
}

void Envelope::onRisingEdge() 
{ 
  m_Envelope->noteOn(); 
  m_TrigOut->on();
  m_LEDColor = sndbx::color::changeBrightness(COLOR, 0.5);
}

void Envelope::onFallingEdge() 
{
  m_Envelope->noteOff(); 
  m_TrigOut->off();
  m_LEDColor = COLOR; 
}

void Envelope::adjustAttack(int delta) 
{
  auto attackCurve = [](float cur, int delta){ return cur + 5*delta; };
  m_Attack.change(attackCurve, delta);
  m_Envelope->attack(m_Attack);
}

void Envelope::adjustDecay(int delta) 
{
  auto decayCurve = [](float cur, int delta){ return cur + 5*delta; };
  m_Decay.change(decayCurve, delta);
  m_Envelope->decay(m_Decay);
}

void Envelope::adjustSustain(int delta) 
{
  auto sustainCurve = [](float cur, int delta){ return cur + 0.05f*delta; };
  m_Sustain.change(sustainCurve, delta);
  m_Envelope->sustain(m_Sustain);
}

void Envelope::adjustRelease(int delta) 
{
  auto releaseCurve = [](float cur, int delta){ return cur + 5*delta; };
  m_Release.change(releaseCurve, delta);
  m_Envelope->release(m_Release);
}
