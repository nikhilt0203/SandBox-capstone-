#ifndef SANDBOX_VCF_HPP_
#define SANDBOX_VCF_HPP_

#include "audio/audio_effect_filter.hpp"
#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

class VCF final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("vcf", "voltage controlled filter", 0xB427F5);
  constexpr static auto max_count = 16U;

public:
  VCF();

  VCF(float cutoff, float resonance, float fmDepth, int filterType);

  void changeControl(std::size_t index, int delta) override;
  void setControls(const Controllable::ControlValues &values) override;
  void resetControls() override;

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
      -> const Controllable::ControlValues & override;

private:
  void cutoffAdjust(int delta);
  void fmDepthAdjust(int delta);
  void resonanceAdjust(int delta);
  void filterTypeAdjust(int delta);
  void initFilter();

protected:
  ModuleParameter<float> m_CutoffFrequency{440.0f, 0.0f, 18000.0f};
  ModuleParameter<float> m_Resonance{1.0f, 0.0f, 5.0f};
  ModuleParameter<float> m_FMDepth{5.0f, 0.0f, 7.0f};
  ModuleParameter<int> m_FilterType{0, 0, 2};

  AudioFilter *m_Filter;

  static inline const Displayable::PortNames m_InputNames{"in", "fm"};
  static inline const Displayable::PortNames m_OutputNames{"out"};
  static inline const Displayable::ControlNames m_ControlNames{"cutoff", "reso",
                                                               "fm", "type"};
  mutable Controllable::ControlValues m_ControlValues;
};

#endif