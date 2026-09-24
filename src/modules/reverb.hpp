#ifndef SANDBOX_REVERB_HPP_
#define SANDBOX_REVERB_HPP_

#include "audio/audio_effect_filter.hpp"
#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

class Reverb final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("reverb", "stereo reverb effect", 0x764589);
  constexpr static auto max_count = 4U;

public:
  Reverb() : Module(1, 2) {
    m_Audio.addDevice<AudioEffectFreeverbStereo>();
    m_Reverb = m_Audio.device<AudioEffectFreeverbStereo>();
    initReverb();

    m_Audio.mapInput(0, m_Reverb, 0);
    m_Audio.mapOutput(0, m_Reverb, 0);
    m_Audio.mapOutput(1, m_Reverb, 1);
  }

  Reverb(float size, float damping) : Reverb() {
    m_RoomSize.setValue(size);
    m_Damping.setValue(damping);
    initReverb();
  }

  void changeControl(std::size_t index, int delta) override {
    switch (index) {
    case 0:
      sizeAdjust(delta);
      break;
    case 1:
      dampingAdjust(delta);
      break;
    default:
      return;
    }
  }

  void setControls(const Controllable::ControlValues &values) override {
    m_RoomSize.denormalize(values.at(0));
    m_Damping.denormalize(values.at(1));
    initReverb();
  }

  void resetControls() override {
    m_RoomSize.reset();
    m_Damping.reset();
    initReverb();
  }

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
      -> const Controllable::ControlValues & override {
    m_ControlValues.clear();
    m_ControlValues.push_back(m_RoomSize.normalized());
    m_ControlValues.push_back(m_Damping.normalized());
    return m_ControlValues;
  }

private:
  void sizeAdjust(int delta) {
    auto sizeCurve = [](float cur, int delta) { return cur + 0.08f * delta; };
    m_RoomSize.change(sizeCurve, delta);
    m_Reverb->roomsize(m_RoomSize);
  }

  void dampingAdjust(int delta) {
    auto dampingCurve = [](float cur, int delta) {
      return cur + 0.08f * delta;
    };
    m_Damping.change(dampingCurve, delta);
    m_Reverb->damping(m_Damping);
  }

  void initReverb() {
    m_Reverb->roomsize(m_RoomSize);
    m_Reverb->damping(m_Damping);
  }

protected:
  ModuleParameter<float> m_RoomSize{0.5f, 0.0f, 1.0f};
  ModuleParameter<float> m_Damping{0.5f, 0.0f, 1.0f};

  AudioEffectFreeverbStereo *m_Reverb{};

  static inline const Displayable::PortNames m_InputNames{"in"};
  static inline const Displayable::PortNames m_OutputNames{"out"};
  static inline const Displayable::ControlNames m_ControlNames{"coarse", "fine",
                                                               "fm", "wave"};
  mutable Controllable::ControlValues m_ControlValues{};
};

#endif