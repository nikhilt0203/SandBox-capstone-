#ifndef SANDBOX_VCF_HPP_
#define SANDBOX_VCF_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_effect_filter.hpp"

class VCF 
: public Module, 
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("vcf", "voltage controlled filter", 0xB427F5);

public:
  VCF();

  VCF(float cutoff, float resonance, float fmDepth, int filterType);

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }

  [[nodiscard]] auto inputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto controlNames() const
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

private:
  void cutoffAdjust(int delta);
  void fmDepthAdjust(int delta);
  void resonanceAdjust(int delta);
  void filterTypeAdjust(int delta);

protected:
  Parameter<float> m_CutoffFrequency{440.0f, 0.0f, 18000.0f};
  Parameter<float> m_Resonance{1.0f, 0.0f, 5.0f};
  Parameter<float> m_FMDepth{5.0f, 0.0f, 7.0f};
  Parameter<int> m_FilterType{0, 0, 2};

  AudioFilter* m_Filter;

  Controls m_Controls{
    [this](int delta){ cutoffAdjust(delta); },
    [this](int delta){ resonanceAdjust(delta); },
    [this](int delta){ fmDepthAdjust(delta); },
    [this](int delta){ filterTypeAdjust(delta); }
  };

  inline static const sndbx::vector_8U<std::string_view> m_InputNames{"in", "fm"};
  inline static const sndbx::vector_8U<std::string_view> m_OutputNames{"out"};
  inline static const sndbx::vector_4U<std::string_view> m_ControlNames{"cutoff", "reso", "fm", "type"};
  mutable sndbx::vector_4U<float> m_ControlValues;
};

#endif