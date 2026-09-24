#ifndef SANDBOX_MIXER_HPP_
#define SANDBOX_MIXER_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

//===================================================
// MIXER
//===================================================

class Mixer : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("mixer", "4 channel mixer", 0x666688);
  constexpr static auto max_count = 32U;

public:
  Mixer();
  Mixer(std::initializer_list<float> gains);

  void changeControl(std::size_t index, int delta) override {
    gainAdjust(index, delta);
  }
  void setControls(const Controllable::ControlValues &values) override;
  void resetControls() override;

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto controlNames() const
      -> const Displayable::ControlNames & override {
    return m_ControlNames;
  }
  [[nodiscard]] auto inputNames() const
      -> const Displayable::PortNames & override {
    return m_InputNames;
  }
  [[nodiscard]] auto outputNames() const
      -> const Displayable::PortNames & override {
    return m_OutputNames;
  }

  [[nodiscard]] auto normalizedControlValues() const
      -> const Controllable::ControlValues & override;

private:
  void gainAdjust(std::size_t channel, int delta);
  void setGains();

private:
  static constexpr std::size_t numChannels = 4U;

  std::array<ModuleParameter<float>, numChannels> m_ChannelGains{
      ModuleParameter<float>{1.0f, 0.0f, 5.0f},
      ModuleParameter<float>{1.0f, 0.0f, 5.0f},
      ModuleParameter<float>{1.0f, 0.0f, 5.0f},
      ModuleParameter<float>{1.0f, 0.0f, 5.0f}};

  AudioMixer4 *m_Mixer{};

  static inline const Displayable::PortNames m_InputNames{"1", "2", "3", "4"};
  static inline const Displayable::PortNames m_OutputNames{"out"};
  static inline const Displayable::ControlNames m_ControlNames{
      "Gain 1", "Gain 2", "Gain 3", "Gain 4"};
  mutable Controllable::ControlValues m_NormalizedControlValues{};
};

#endif