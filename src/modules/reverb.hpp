#ifndef SANDBOX_REVERB_HPP_
#define SANDBOX_REVERB_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
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
    initReverb();

    m_Audio.mapInput(0, m_Reverb, 0);
    m_Audio.mapOutput(0, m_Reverb, 0);
    m_Audio.mapOutput(1, m_Reverb, 1);
  }

  Reverb(float size, float damping) : Reverb()
  {
    m_RoomSize.setValue(size);
    m_Damping.setValue(damping);
    initReverb();
  }

  void changeControl(std::size_t index, int delta) override
  {
    switch (index)
    {
      case 0: sizeAdjust(delta); break;
      case 1: dampingAdjust(delta); break;
      default: return;
    }
  }
  
  void resetControls() override
  {
    m_RoomSize.reset();
    m_Damping.reset();
    initReverb();
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto inputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_InputNames; }
  
  [[nodiscard]] auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override
  { 
    m_ControlValues.clear();
    m_ControlValues.push_back(m_RoomSize.normalized());
    m_ControlValues.push_back(m_Damping.normalized());
    return m_ControlValues;
  }

private:
  void sizeAdjust(int delta) 
  {
    auto sizeCurve = [](float cur, int delta){ return cur + 0.08f*delta; };
    m_RoomSize.change(sizeCurve, delta);
    m_Reverb->roomsize(m_RoomSize);
  }

  void dampingAdjust(int delta) 
  {
    auto dampingCurve = [](float cur, int delta){ return cur + 0.08f*delta; };
    m_Damping.change(dampingCurve, delta);
    m_Reverb->damping(m_Damping);
  }

  void initReverb()
  {
    m_Reverb->roomsize(m_RoomSize);
    m_Reverb->damping(m_Damping);
  }

protected:
  ModuleParameter<float> m_RoomSize{0.5f, 0.0f, 1.0f};
  ModuleParameter<float> m_Damping{0.5f, 0.0f, 1.0f};

  AudioEffectFreeverbStereo* m_Reverb{};

  inline static const sndbx::vector_8U<std::string_view> m_InputNames{"in"};
  inline static const sndbx::vector_8U<std::string_view> m_OutputNames{"out"};
  inline static const sndbx::vector_4U<std::string_view> m_ControlNames{"coarse", "fine", "fm", "wave"};
  mutable sndbx::vector_4U<float> m_ControlValues{};
};

#endif