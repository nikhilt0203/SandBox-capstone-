#include "vcf.hpp"

VCF::VCF() : Module(2, 1)
{
  m_Audio.addDevice<AudioFilter>();
  m_Filter = m_Audio.device<AudioFilter>();

  m_Filter->frequency(m_CutoffFrequency);
  m_Filter->resonance(m_Resonance);
  m_Filter->octaveControl(m_FMDepth);

  m_Audio.mapInput(0, m_Filter, 0);
  m_Audio.mapInput(1, m_Filter, 1);
  m_Audio.mapOutput(0, m_Filter, 0);
}

VCF::VCF(float cutoff, float resonance, float fmDepth, int filterType) 
  : VCF()
{
  m_CutoffFrequency.setValue(cutoff);
  m_Resonance.setValue(resonance);
  m_FMDepth.setValue(fmDepth);
  m_FilterType.setValue(filterType);

  m_Filter->frequency(m_CutoffFrequency);
  m_Filter->resonance(m_Resonance);
  m_Filter->octaveControl(m_FMDepth);
  m_Filter->filterType(m_FilterType);
}

auto VCF::normalizedControlValues() const -> const sndbx::vector_4U<float>&
{ 
  m_ControlValues.clear();
  m_ControlValues.push_back(m_CutoffFrequency.normalized());
  m_ControlValues.push_back(m_Resonance.normalized());
  m_ControlValues.push_back(m_FMDepth.normalized());
  m_ControlValues.push_back(m_FilterType.normalized());
  return m_ControlValues;
}

void VCF::cutoffAdjust(int delta) 
{
  auto freqCurve = [](float cur, int delta){ return cur * powf(1.08f, 1*delta); };
  m_CutoffFrequency.change(freqCurve, delta);
  m_Filter->frequency(m_CutoffFrequency);
}

void VCF::fmDepthAdjust(int delta) 
{
  auto fmCurve = [](float cur, int delta){ return cur + 0.25f*delta; };
  m_FMDepth.change(fmCurve, delta);
  m_Filter->octaveControl(m_FMDepth);
}

void VCF::resonanceAdjust(int delta) 
{
  auto resoCurve = [](float cur, int delta){ return cur + 1*delta; };
  m_Resonance.change(resoCurve, delta);
  m_Filter->resonance(m_Resonance);
}

void VCF::filterTypeAdjust(int delta) 
{
  auto curve = [](int cur, int delta){ return (cur + delta) % 3; };
  m_FilterType.change(curve, delta);
  m_Filter->filterType(m_FilterType);
}