#ifndef SANDBOX_ENVELOPE_HPP_
#define SANDBOX_ENVELOPE_HPP_

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

  Envelope(float attack, float decay, float sustain, float release)
    : Envelope()
  {
    m_Attack.setValue(attack);
    m_Decay.setValue(decay);
    m_Sustain.setValue(sustain);
    m_Release.setValue(release);

    m_Envelope->attack(m_Attack);
    m_Envelope->decay(m_Decay);
    m_Envelope->sustain(m_Sustain);
    m_Envelope->release(m_Release);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  void resetControls() override;
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }

  [[nodiscard]] auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }
    
  [[nodiscard]] auto inputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;

  void onRisingEdge() override;
  void onFallingEdge() override;

private:
  void adjustAttack(int delta);
  void adjustDecay(int delta);
  void adjustSustain(int delta);
  void adjustRelease(int delta);
  void initEnvelope();

private:
  ModuleParameter<float> m_Attack{10.0f, 0.0f, 500.0f};
  ModuleParameter<float> m_Decay{35.0f, 0.0f, 500.0f};
  ModuleParameter<float> m_Sustain{0.0f, 0.0f, 1.0f};
  ModuleParameter<float> m_Release{100.0f, 0.0f, 500.0f};

  Controls m_Controls{
    [this](int delta){ adjustAttack(delta); },
    [this](int delta){ adjustDecay(delta); },
    [this](int delta){ adjustSustain(delta); },
    [this](int delta){ adjustRelease(delta); }
  };

  AudioEffectEnvelope* m_Envelope;
  AudioTriggerOutput* m_TrigOut;

  std::uint32_t m_LEDColor{COLOR};

  inline static const sndbx::vector_8U<std::string_view> m_InputNames{"in", "trg"};
  inline static const sndbx::vector_8U<std::string_view> m_OutputNames{"out", "trg"};
  inline static const sndbx::vector_4U<std::string_view> m_ControlNames{"attack", "decay", "sustain", "release"};
  mutable sndbx::vector_4U<float> m_ControlValues{};
};

#endif