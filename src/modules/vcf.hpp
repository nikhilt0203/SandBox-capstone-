#ifndef vcf_hpp_
#define vcf_hpp_

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
  VCF() : Module(2, 1)
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

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override 
  {
    static const std::vector<std::string_view> controlNames{
      "cutoff", "reso", "fm", "type"
    };
    return controlNames;
  }

  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override
  { 
    static std::vector<float> values;
    values.reserve(4);
    values.clear();
    values.push_back(m_CutoffFrequency.normalized());
    values.push_back(m_Resonance.normalized());
    values.push_back(m_FMDepth.normalized());
    values.push_back(m_FilterType.normalized());
    return values;
  }

  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override 
  {
    static const std::vector<std::string_view> inputNames{"in", "fm"};
    return inputNames;
  }

  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override 
  {
    static const std::vector<std::string_view> outputNames{"out"};
    return outputNames;
  }

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

private:
  void cutoffAdjust(int delta) 
  {
    auto freqCurve = [](float cur, int delta){ return cur * powf(1.08f, 1*delta); };
    m_CutoffFrequency.change(freqCurve, delta);
    m_Filter->frequency(m_CutoffFrequency);
  }

  void fmDepthAdjust(int delta) 
  {
    auto fmCurve = [](float cur, int delta){ return cur + 0.25f*delta; };
    m_FMDepth.change(fmCurve, delta);
    m_Filter->octaveControl(m_FMDepth);
  }

  void resonanceAdust(int delta) 
  {
    auto resoCurve = [](float cur, int delta){ return cur + 1*delta; };
    m_Resonance.change(resoCurve, delta);
    m_Filter->resonance(m_Resonance);
  }

  void filterTypeAdjust(int delta) 
  {
    auto curve = [](int cur, int delta){ return (cur + delta) % 3; };
    m_FilterType.change(curve, delta);
    m_Filter->filterType(m_FilterType);
  }

protected:
  Parameter<float> m_CutoffFrequency{440.0f, 0.0f, 18000.0f};
  Parameter<float> m_Resonance{1.0f, 0.0f, 5.0f};
  Parameter<float> m_FMDepth{5.0f, 0.0f, 7.0f};
  Parameter<int> m_FilterType{0, 0, 2};

  AudioFilter* m_Filter;

  Controls m_Controls{
    [this](int delta){ cutoffAdjust(delta); },
    [this](int delta){ resonanceAdust(delta); },
    [this](int delta){ fmDepthAdjust(delta); },
    [this](int delta){ filterTypeAdjust(delta); }
  };
};

#endif