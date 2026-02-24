#ifndef envelope_hpp_
#define envelope_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_trigger_input.hpp"
#include "audio/audio_trigger_output.hpp"
#include "ui/color.hpp"

class Envelope 
: public Module, 
  public Controllable,
  public Displayable,
  public Pressable
{
public:
  MODULE_TYPE_INFO("envelope", "applies an envelope to the input signal", 0xFF7300);

public:
  Envelope();

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override { return m_ControlNames; }
  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override { return m_InputNames; }
  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override { return m_OutputNames; }
  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override;

  
  void onRisingEdge() override;
  void onFallingEdge() override;

private:
  void adjustAttack(int delta);
  void adjustDecay(int delta);
  void adjustSustain(int delta);
  void adjustRelease(int delta);

private:
  Controls m_Controls{
    [this](int delta){ adjustAttack(delta); },
    [this](int delta){ adjustDecay(delta); },
    [this](int delta){ adjustSustain(delta); },
    [this](int delta){ adjustRelease(delta); }
  };

  Parameter<float> m_Attack{10.0f, 0.0f, 500.0f};
  Parameter<float> m_Decay{35.0f, 0.0f, 500.0f};
  Parameter<float> m_Sustain{0.0f, 0.0f, 1.0f};
  Parameter<float> m_Release{200.0f, 0.0f, 500.0f};

  AudioEffectEnvelope* m_Envelope;
  AudioTriggerOutput* m_TrigOut;

  std::uint32_t m_LEDColor{displayColor()};

  inline static const std::vector<std::string_view> m_ControlNames{"attack", "decay", "sustain", "release"};
  inline static const std::vector<std::string_view> m_InputNames{"in", "trg"};
  inline static const std::vector<std::string_view> m_OutputNames{"out", "trg"};
  mutable std::vector<float> m_ControlValues{};
};

#endif