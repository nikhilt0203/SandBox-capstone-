#ifndef SANDBOX_SEQUENCER_HPP_
#define SANDBOX_SEQUENCER_HPP_

#include "audio/audio_trigger_input.hpp"
#include "audio/audio_trigger_output.hpp"
#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

class SequencerStep;
class Sequencer final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("sequencer", "", 0x367591);
  constexpr static auto max_count = 5U;

  using LengthChangeCallback = bool (*)(Sequencer &);

public:
  Sequencer();

  void changeControl(std::size_t index, int delta) override;
  void setControls(const Controllable::ControlValues &values) override;
  void resetControls() override;

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto inputNames() const
      -> const Displayable::PortNames & override {
    return m_InputNames;
  }

  [[nodiscard]] auto outputNames() const
      -> const Displayable::PortNames & override {
    return m_OutputNames;
  }

  [[nodiscard]] auto controlNames() const
      -> const Displayable::ControlNames & override {
    return m_ControlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const
      -> const Controllable::ControlValues & override;

  [[nodiscard]] std::size_t numSteps() const { return m_NumSteps; }

  void nextStep();

  void setAddStepCallback(LengthChangeCallback cb) { m_AddStepCallback = cb; }
  void setSubtractStepCallback(LengthChangeCallback cb) {
    m_SubtractStepCallback = cb;
  }

  void registerStep(SequencerStep *step) { m_Steps.push_back(step); }

private:
  void lengthAdjust(int delta);
  void thresholdAdjust(int delta);

private:
  ModuleParameter<std::size_t> m_NumSteps{0U, 1U, 32U};
  ModuleParameter<float> m_TriggerThreshold{0.25f, 0.0f, 1.0f};

  LengthChangeCallback m_AddStepCallback{};
  LengthChangeCallback m_SubtractStepCallback{};

  AudioSynthWaveformDc *m_DC{};
  AudioTriggerOutput *m_TrigOut{};
  AudioTriggerInput *m_TrigIn{};

  nst::vector_32U<SequencerStep *> m_Steps{};
  nst::vector_32U<float> m_StepMemory{};
  std::size_t m_CurrentStep{};

  static inline Displayable::PortNames m_InputNames{"trg"};
  static inline Displayable::PortNames m_OutputNames{"cv", "trg"};
  static inline Displayable::ControlNames m_ControlNames{"length", "thresh"};
  mutable Controllable::ControlValues m_ControlValues{};
};

class SequencerStep final : public Module,
                            public Displayable,
                            public Controllable {
public:
  MODULE_TYPE_INFO("step", "", 0x2222FF);
  constexpr static auto max_count = 54U;

public:
  SequencerStep() : Module(0, 0) {}

  void changeControl(std::size_t index, int delta) override {
    if (index == 0) {
      adjustAmplitude(delta);
    }
  }
  void setControls(const Controllable::ControlValues &values) {
    m_Amplitude.setValue(values.at(0));
  }
  void resetControls() override { m_Amplitude.reset(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }

  [[nodiscard]] auto controlNames() const
      -> const Displayable::ControlNames & override {
    return m_ControlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const
      -> const Controllable::ControlValues & override;

  [[nodiscard]] float amplitude() const { return m_Amplitude; }

  [[nodiscard]] std::uint32_t parentID() { return m_ParentID; }

  void setAmplitude(float amplitude);

  void setParentID(std::uint32_t parentID) { m_ParentID = parentID; }

  void on() { m_LEDColor = 0xFF0000; }
  void off() { m_LEDColor = m_CurrentColor; }

private:
  void adjustAmplitude(int delta);
  void updateColor();

private:
  ModuleParameter<float> m_Amplitude{0.0f, 0.0f, 1.0f};

  std::uint32_t m_ParentID{};

  std::uint32_t m_LEDColor{COLOR};
  std::uint32_t m_CurrentColor{COLOR};

  static inline Displayable::ControlNames m_ControlNames{"amplitude"};
  mutable Controllable::ControlValues m_ControlValues{};
};

#endif