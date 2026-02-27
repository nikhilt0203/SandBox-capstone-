#ifndef SANDBOX_REVERB_HPP_
#define SANDBOX_REVERB_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_effect_filter.hpp"

class Reverb 
: public Module, 
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("reverb", "stereo reverb effect", 0x764589);

public:
  Reverb() : Module(1, 2)
  {
    m_Audio.addDevice<AudioEffectFreeverbStereo>();
    m_Reverb = m_Audio.device<AudioEffectFreeverbStereo>();

    m_Reverb->roomsize(m_Size);
    m_Reverb->damping(m_Damping);

    m_Audio.mapInput(0, m_Reverb, 0);
    m_Audio.mapOutput(0, m_Reverb, 0);
    m_Audio.mapOutput(1, m_Reverb, 1);
  }

  Reverb(float size, float damping) : Reverb()
  {
    m_Size.setValue(size);
    m_Damping.setValue(damping);
    m_Reverb->roomsize(m_Size);
    m_Reverb->damping(m_Damping);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
   [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }


  [[nodiscard]] auto controlNames() const 
    -> const std::vector<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto inputNames() const 
    -> const std::vector<std::string_view>& override { return m_InputNames; }
  

  [[nodiscard]] auto outputNames() const 
    -> const std::vector<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const std::vector<float>& override
  { 
    m_ControlValues.reserve(2);
    m_ControlValues.clear();
    m_ControlValues.push_back(m_Size.normalized());
    m_ControlValues.push_back(m_Damping.normalized());
    return m_ControlValues;
  }


private:
  void sizeAdjust(int delta) 
  {
    auto sizeCurve = [](float cur, int delta){ return cur + 0.08f*delta; };
    m_Size.change(sizeCurve, delta);
    m_Reverb->roomsize(m_Size);
  }

  void dampingAdjust(int delta) 
  {
    auto dampingCurve = [](float cur, int delta){ return cur + 0.08f*delta; };
    m_Damping.change(dampingCurve, delta);
    m_Reverb->damping(m_Damping);
  }

protected:
  Parameter<float> m_Size{0.5f, 0.0f, 1.0f};
  Parameter<float> m_Damping{0.5f, 0.0f, 1.0f};

  AudioEffectFreeverbStereo* m_Reverb{};

  Controls m_Controls{
    [this](int delta){ sizeAdjust(delta); },
    [this](int delta){ dampingAdjust(delta); }
  };

  inline static const std::vector<std::string_view> m_ControlNames{"coarse", "fine", "fm", "wave"};
  inline static const std::vector<std::string_view> m_InputNames{"in"};
  inline static const std::vector<std::string_view> m_OutputNames{"out"};
  mutable std::vector<float> m_ControlValues{};
};

#endif