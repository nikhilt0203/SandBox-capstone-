#ifndef envelope_hpp_
#define envelope_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_trigger_input.hpp"
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
  Envelope() : Module(2, 1)
  {
    m_Audio.addDevice<AudioEffectEnvelope>();
    m_Envelope = m_Audio.device<AudioEffectEnvelope>();

    m_Envelope->attack(m_Attack);
    m_Envelope->decay(m_Decay);
    m_Envelope->sustain(m_Sustain);
    m_Envelope->release(m_Release);

    m_Audio.mapInput(0, m_Envelope, 0);
    m_Audio.mapOutput(0, m_Envelope, 0);

    m_Audio.addDevice<AudioTriggerInput>();
    auto trigIn = m_Audio.device<AudioTriggerInput>(1);
    trigIn->risingEdgeCallback([this](){ onRisingEdge(); });
    trigIn->fallingEdgeCallback([this](){ onFallingEdge(); });
    m_Audio.mapInput(1, trigIn, 0);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override 
  {
    static const std::vector<std::string_view> controlNames{
      "attack", "decay", "sustain", "release"
    };
    return controlNames;
  }

  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override
  { 
    static std::vector<float> values;
    values.reserve(4);
    values.clear();
    values.push_back(m_Attack.normalized());
    values.push_back(m_Decay.normalized());
    values.push_back(m_Sustain.normalized());
    values.push_back(m_Release.normalized());
    return values;
  }

  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override 
  {
    static const std::vector<std::string_view> inputNames{"in", "trg"};
    return inputNames;
  }

  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override 
  {
    static const std::vector<std::string_view> outputNames{"out", "trg"};
    return outputNames;
  }

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }
  
  void onRisingEdge() override 
  { 
    constexpr static auto pressedColor = sndbx::color::changeBrightness(COLOR, 0.5);
    m_Envelope->noteOn(); 
    m_LEDColor = pressedColor;
  }

  void onFallingEdge() override 
  { 
    m_Envelope->noteOff(); 
    m_LEDColor = COLOR; 
  }

private:
  void adjustAttack(int delta) 
  {
    auto attackCurve = [](float cur, int delta){ return cur + 5*delta; };
    m_Attack.change(attackCurve, delta);
    m_Envelope->attack(m_Attack);
  }

  void adjustDecay(int delta) 
  {
    auto decayCurve = [](float cur, int delta){ return cur + 5*delta; };
    m_Decay.change(decayCurve, delta);
    m_Envelope->decay(m_Decay);
  }

  void adjustSustain(int delta) 
  {
    auto sustainCurve = [](float cur, int delta){ return cur + 0.05f*delta; };
    m_Sustain.change(sustainCurve, delta);
    m_Envelope->sustain(m_Sustain);
  }

  void adjustRelease(int delta) 
  {
    auto releaseCurve = [](float cur, int delta){ return cur + 5*delta; };
    m_Release.change(releaseCurve, delta);
    m_Envelope->release(m_Release);
  }

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

  std::uint32_t m_LEDColor{displayColor()};
};

#endif