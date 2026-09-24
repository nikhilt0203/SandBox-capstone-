#ifndef SANDBOX_COMBINE_HPP_
#define SANDBOX_COMBINE_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

class Combine final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("combine", "digital combine", 0xFF4589);
  constexpr static auto max_count = 16U;

public:
  Combine() : Module(2, 1) {
    m_Audio.addDevice<AudioEffectDigitalCombine>();
    m_Combine = m_Audio.device<AudioEffectDigitalCombine>();

    m_Audio.mapInput(0, m_Combine, 0);
    m_Audio.mapInput(1, m_Combine, 1);
    m_Audio.mapOutput(0, m_Combine, 0);
  }

  void changeControl(std::size_t index, int delta) override {
    if (index != 0) {
      return;
    }
    modeAdjust(delta);
  }

  void setControls(const Controllable::ControlValues &values) override {
    m_Mode.denormalize(values.at(0));
    m_Combine->setCombineMode(m_Mode);
  }

  void resetControls() override {
    m_Mode.reset();
    m_Combine->setCombineMode(m_Mode);
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
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
      -> const Controllable::ControlValues & override {
    m_ControlValues.clear();
    m_ControlValues.push_back(m_Mode.normalized());
    return m_ControlValues;
  }

private:
  void modeAdjust(int delta) {
    auto modeCurve = [](std::size_t cur, int delta) { return cur + delta; };
    m_Mode.change(modeCurve, delta);
    m_Combine->setCombineMode(m_Mode);
  }

protected:
  ModuleParameter<std::size_t> m_Mode{0, 0, 3};

  AudioEffectDigitalCombine *m_Combine{};

  static inline const Displayable::PortNames m_InputNames{"1", "2"};
  static inline const Displayable::PortNames m_OutputNames{"out"};
  static inline const Displayable::ControlNames m_ControlNames{"mode"};
  mutable Controllable::ControlValues m_ControlValues{};
};

#endif