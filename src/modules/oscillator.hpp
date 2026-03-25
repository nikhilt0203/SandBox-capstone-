#ifndef SANDBOX_OSCILLATOR_HPP_
#define SANDBOX_OSCILLATOR_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "serialization.hpp"

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
  Oscillator(float frequency, int fineTuneOffset, float fmDepth, std::size_t waveform);

  void changeControl(std::size_t index, int delta) override;
  void resetControls() override;
  [[nodiscard]] std::size_t numControls() const override { return 4U; }

  [[nodiscard]] std::string_view displayName() const override { return m_Waveforms.at(m_WaveformIndex).name; }
  [[nodiscard]] std::uint32_t displayColor() const override { return m_Waveforms.at(m_WaveformIndex).color; }

  [[nodiscard]] auto inputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;
  
private:
  void frequencyAdjustCoarse(int delta);
  void frequencyAdjustFine(int delta);
  void fmDepthAdjust(int delta);
  void waveformAdjust(int delta);
  void initSynthWaveform();

protected:
  AudioSynthWaveformModulated* m_Oscillator{};

  ModuleParameter<float> m_Frequency{440.0f, 0.0f, 18000.0f};
  ModuleParameter<int> m_FineTuneOffset{0, -50, 50};
  ModuleParameter<float> m_FMDepth{8.25f, 0.0f, 12.0f};
  ModuleParameter<std::size_t> m_WaveformIndex{0U, 0U, numWaveforms - 1};

  struct Waveform
  {
    short id;
    std::string_view name;
    std::uint32_t color;
  };

  static constexpr std::size_t numWaveforms = 7U;
  static const std::array<Waveform, numWaveforms> m_Waveforms;

private:
  inline static const sndbx::vector_8U<std::string_view> m_InputNames{"fm", "wv"};
  inline static const sndbx::vector_8U<std::string_view> m_OutputNames{"out"};
  inline static const sndbx::vector_4U<std::string_view> m_ControlNames{"coarse", "fine", "fm", "wave"};
  mutable sndbx::vector_4U<float> m_ControlValues{};
};

#endif