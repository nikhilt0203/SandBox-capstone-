#include "sequencer.hpp"
#include "ui/color.hpp"

Sequencer::Sequencer() : Module(1, 2) {
  m_Audio.addDevice<AudioSynthWaveformDc>();
  m_Audio.addDevice<AudioTriggerOutput>();
  m_Audio.addDevice<AudioTriggerInput>();

  m_DC = m_Audio.device<AudioSynthWaveformDc>();
  m_TrigOut = m_Audio.device<AudioTriggerOutput>(1);
  m_TrigIn = m_Audio.device<AudioTriggerInput>(2);

  m_TrigIn->risingEdgeCallback(
      nst::method_ref::bind<Sequencer, &Sequencer::nextStep>(this));

  m_Audio.mapInput(0, m_TrigIn, 0);
  m_Audio.mapOutput(0, m_DC, 0);
  m_Audio.mapOutput(1, m_TrigOut, 0);
}

void Sequencer::changeControl(std::size_t index, int delta) {
  switch (index) {
  case 0:
    lengthAdjust(delta);
    break;
  case 1:
    thresholdAdjust(delta);
    break;
  default:
    return;
  }
}

void Sequencer::resetControls() {
  m_NumSteps.reset();
  m_TriggerThreshold.reset();
}

void Sequencer::setControls(const Controllable::ControlValues &values) {
  m_NumSteps.denormalize(values.at(0));
  m_TriggerThreshold.denormalize(values.at(1));
}

auto Sequencer::normalizedControlValues() const
    -> const Controllable::ControlValues & {
  m_ControlValues.clear();
  m_ControlValues.push_back(m_NumSteps.normalized());
  m_ControlValues.push_back(m_TriggerThreshold);
  return m_ControlValues;
}

void Sequencer::nextStep() {
  const std::size_t numSteps = m_Steps.size();
  if (numSteps == 0) {
    return;
  }

  if (m_CurrentStep >= numSteps) {
    m_CurrentStep = 0;
  }
  if (m_CurrentStep >= m_NumSteps) {
    m_CurrentStep = 0;
  }
  const auto currentIndex = m_CurrentStep;

  const auto previousIndex =
      (currentIndex == 0) ? numSteps - 1 : currentIndex - 1;
  auto previousStep = m_Steps[previousIndex];
  auto currentStep = m_Steps[currentIndex];

  const auto currentAmplitude = currentStep->amplitude();

  m_DC->amplitude(currentAmplitude);
  (currentAmplitude > 0.0f) ? m_TrigOut->on() : m_TrigOut->off();

  currentStep->on();
  previousStep->off();

  m_StepMemory[currentIndex] = currentAmplitude;
  m_CurrentStep = (currentIndex + 1 == numSteps) ? 0 : currentIndex + 1;
}

void Sequencer::lengthAdjust(int delta) {
  auto lenCurve = [](std::size_t cur, int delta) { return cur + 1 * delta; };

  if (!m_AddStepCallback || !m_SubtractStepCallback) {
    return;
  }

  bool addedStep = false;
  bool subtractedStep = false;
  if (delta > 0 && m_NumSteps < m_NumSteps.max()) {
    addedStep = m_AddStepCallback(*this);
  }
  if (delta < 0 && m_NumSteps > m_NumSteps.min()) {
    subtractedStep = m_SubtractStepCallback(*this);
  }

  if (addedStep || subtractedStep) {
    m_NumSteps.change(lenCurve, delta);
  }
  if (subtractedStep) {
    m_Steps.pop_back();
  }
}

void Sequencer::thresholdAdjust(int delta) {
  auto thresholdCurve = [](float v, int d) -> float { return v + 0.025 * d; };
  m_TriggerThreshold.change(thresholdCurve, delta);
  m_TrigIn->threshold(m_TriggerThreshold);
}

auto SequencerStep::normalizedControlValues() const
    -> const Controllable::ControlValues & {
  m_ControlValues.clear();
  m_ControlValues.push_back(m_Amplitude);
  return m_ControlValues;
}

void SequencerStep::updateColor() {
  m_CurrentColor = sndbx::color::blend(COLOR, 0xDD0FFF, m_Amplitude);
  m_LEDColor = m_CurrentColor;
}

void SequencerStep::adjustAmplitude(int delta) {
  auto amplitudeCurve = [](float cur, int delta) {
    return cur + 0.01f * delta;
  };
  m_Amplitude.change(amplitudeCurve, delta);
  updateColor();
}

void SequencerStep::setAmplitude(float amplitude) {
  m_Amplitude.value() =
      std::clamp(amplitude, m_Amplitude.min(), m_Amplitude.max());
  updateColor();
}
