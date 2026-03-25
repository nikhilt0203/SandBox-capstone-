#ifndef SANDBOX_MIXER_HPP_
#define SANDBOX_MIXER_HPP_

#include "dep/module.hpp"
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

  Mixer(std::initializer_list<float> gains);

  void changeControl(std::size_t index, int delta) override { gainAdjust(index, delta); }
  void resetControls() override;

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  
  [[nodiscard]] auto controlNames() const
     -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto inputNames() const
     -> const sndbx::vector_8U<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const
     -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;

private:
  void gainAdjust(std::size_t channel, int delta);
  void setGains();

private:
  static constexpr std::size_t numChannels = 4U;

  std::array<ModuleParameter<float>, numChannels> m_ChannelGains{ 
    ModuleParameter<float>{1.0f, 0.0f, 5.0f}, 
    ModuleParameter<float>{1.0f, 0.0f, 5.0f}, 
    ModuleParameter<float>{1.0f, 0.0f, 5.0f}, 
    ModuleParameter<float>{1.0f, 0.0f, 5.0f} 
  };

  Controls m_Controls{
    [this](int delta){ gainAdjust(0, delta); },
    [this](int delta){ gainAdjust(1, delta); },
    [this](int delta){ gainAdjust(2, delta); },
    [this](int delta){ gainAdjust(3, delta); }
  };

  AudioMixer4* m_Mixer{};

  inline static const sndbx::vector_8U<std::string_view> m_InputNames{"1", "2", "3", "4"};
  inline static const sndbx::vector_8U<std::string_view> m_OutputNames{"out"};
  inline static const sndbx::vector_4U<std::string_view> m_ControlNames{"Gain 1", "Gain 2", "Gain 3", "Gain 4"};
  mutable sndbx::vector_4U<float> m_NormalizedControlValues{};
};

#endif