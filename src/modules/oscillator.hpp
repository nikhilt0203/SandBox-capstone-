#ifndef oscillator_hpp_
#define oscillator_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"

//===================================================
// OSCILLATOR
//===================================================

class Oscillator 
: public Module, 
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("oscillator", "outputs a continuous waveform", 0x00FF00);

public:
  Oscillator();

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return m_Waveforms.at(m_WaveformIndex).name; }
  [[nodiscard]] std::uint32_t displayColor() const override { return m_Waveforms.at(m_WaveformIndex).color; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override { return m_ControlNames; }
  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override { return m_InputNames; }
  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override { return m_OutputNames; }
  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override { return m_ControlValues; }

private:
  void frequencyAdjustCoarse(int delta);
  void frequencyAdjustFine(int delta);
  void fmDepthAdjust(int delta);
  void waveformAdjust(int delta);

protected:
  AudioSynthWaveformModulated* m_Oscillator;

  Parameter<float> m_Frequency{440.0f, 0.0f, 18000.0f};
  Parameter<int> m_FineTuneOffset{0, -50, 50};
  Parameter<float> m_FMDepth{5.0f, 0.0f, 12.0f};
  Parameter<std::size_t> m_WaveformIndex{0U, 0U, numWaveforms - 1};

  Controls m_Controls{
    [this](int delta){ frequencyAdjustCoarse(delta); },
    [this](int delta){ frequencyAdjustFine(delta); },
    [this](int delta){ fmDepthAdjust(delta); },
    [this](int delta){ waveformAdjust(delta); }
  };

  struct Waveform
  {
    short id;
    std::string_view name;
    std::uint32_t color;
  };

  static constexpr std::size_t numWaveforms = 7U;
  static std::array<Waveform, numWaveforms> m_Waveforms;

private:
  inline static const std::vector<std::string_view> m_ControlNames{"coarse", "fine", "fm", "wave"};
  inline static const std::vector<std::string_view> m_InputNames{"fm", "wv"};
  inline static const std::vector<std::string_view> m_OutputNames{"out"};
  mutable std::vector<float> m_ControlValues{};
};

#endif