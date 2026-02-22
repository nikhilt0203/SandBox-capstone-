#ifndef mixer_hpp_
#define mixer_hpp_

#include "dep/module.hpp"
#include "dep/controls.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

//===================================================
// MIXER
//===================================================

class Mixer 
: public Module, 
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("mixer", "4 channel mixer", 0x666688);

public:
  Mixer();

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }
  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override { return m_ControlNames; }
  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override { return m_InputNames; }
  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override { return m_OutputNames; }
  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override;

private:
  void gainAdjust(std::size_t channel, int delta);

private:
  static constexpr std::size_t numChannels = 4U;

  std::array<Parameter<float>, numChannels> m_ChannelGains{ 
    Parameter<float>{1.0f, 0.0f, 5.0f}, 
    Parameter<float>{1.0f, 0.0f, 5.0f}, 
    Parameter<float>{1.0f, 0.0f, 5.0f}, 
    Parameter<float>{1.0f, 0.0f, 5.0f} 
  };

  Controls m_Controls{
    [this](int delta){ gainAdjust(0, delta); },
    [this](int delta){ gainAdjust(1, delta); },
    [this](int delta){ gainAdjust(2, delta); },
    [this](int delta){ gainAdjust(3, delta); }
  };

  AudioMixer4* m_Mixer{};

private:
  inline static const std::vector<std::string_view> m_ControlNames{"Gain 1", "Gain 2", "Gain 3", "Gain 4"};
  inline static const std::vector<std::string_view> m_InputNames{"1", "2", "3", "4"};
  inline static const std::vector<std::string_view> m_OutputNames{"out"};
  mutable std::vector<float> m_NormalizedControlValues{};
};

#endif