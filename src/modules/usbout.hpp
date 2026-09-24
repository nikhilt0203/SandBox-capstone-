#ifndef SANDBOX_USBOUT_HPP_
#define SANDBOX_USBOUT_HPP_

#include "audio/output_i2smixer.hpp"
#include "audio/output_usbmixer.hpp"
#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"

//===================================================
// USBOUT
//===================================================
class I2SOut final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("audio out", "outputs audio via speakers", 0x0000FF);
  constexpr static auto max_count = 1U;

public:
  I2SOut() : Module(1, 0) {
    enableAudioShield();
    m_Audio.addDevice<AudioOutputI2SMixer>();
    m_OutputMixer = m_Audio.device<AudioOutputI2SMixer>();
    m_Audio.mapInput(0, m_OutputMixer, 0);
  }

  void changeControl(std::size_t index, int delta) override {
    if (index == 0) {
      volumeAdjust(delta);
    }
  }
  void setControls(const Controllable::ControlValues &values) override {
    m_Volume.denormalize(values.at(0));
    m_OutputMixer->volume(m_Volume);
  }
  void resetControls() {
    m_Volume.reset();
    m_OutputMixer->volume(m_Volume);
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; };

  [[nodiscard]] auto inputNames() const
      -> const Displayable::PortNames & override {
    return m_InputNames;
  }

  [[nodiscard]] auto controlNames() const
      -> const Displayable::ControlNames & override {
    return m_ControlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const
      -> const Controllable::ControlValues & override {
    m_ControlValues.clear();
    m_ControlValues.push_back(m_Volume.normalized());
    return m_ControlValues;
  }

private:
  void volumeAdjust(int delta) {
    auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1 * d); };
    m_Volume.change(gainCurve, delta);
    m_OutputMixer->volume(m_Volume);
  }

private:
  ModuleParameter<float> m_Volume{0.5f, 0.0f, 2.0f};

  AudioOutputI2SMixer *m_OutputMixer{};

private:
  static inline const Displayable::PortNames m_InputNames{"in"};
  static inline const Displayable::ControlNames m_ControlNames{"volume"};
  mutable Controllable::ControlValues m_ControlValues;

  static inline AudioControlSGTL5000 m_AudioShield{};

  static void enableAudioShield() {
    static bool enabled = false;
    if (enabled) {
      return;
    }
    m_AudioShield.enable();
    m_AudioShield.volume(0.5);
    enabled = true;
  }
};

// USB OUT
class USBOut final : public Module, public Controllable, public Displayable {
public:
  MODULE_TYPE_INFO("audio out", "outputs audio via usb or speakers", 0x0000FF);
  constexpr static auto max_count = 1U;

public:
  USBOut() : Module(1, 0) {
    m_Audio.addDevice<AudioOutputUSBMixer>();
    m_OutputMixer = m_Audio.device<AudioOutputUSBMixer>();
    m_Audio.mapInput(0, m_OutputMixer, 0);
  }

  void changeControl(std::size_t index, int delta) override {
    if (index == 0) {
      volumeAdjust(delta);
    }
  }
  void setControls(const Controllable::ControlValues &values) override {
    m_Volume.denormalize(values.at(0));
    m_OutputMixer->volume(m_Volume);
  }
  void resetControls() {
    m_Volume.reset();
    m_OutputMixer->volume(m_Volume);
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; };

  [[nodiscard]] auto inputNames() const
      -> const Displayable::PortNames & override {
    return m_InputNames;
  }

  [[nodiscard]] auto controlNames() const
      -> const Displayable::ControlNames & override {
    return m_ControlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const
      -> const Controllable::ControlValues & override {
    m_ControlValues.clear();
    m_ControlValues.push_back(m_Volume.normalized());
    return m_ControlValues;
  }

private:
  void volumeAdjust(int delta) {
    auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1 * d); };
    m_Volume.change(gainCurve, delta);
    m_OutputMixer->volume(m_Volume);
  }

private:
  ModuleParameter<float> m_Volume{1.0f, 0.0f, 2.0f};

  AudioOutputUSBMixer *m_OutputMixer{};

  static inline const Displayable::PortNames m_InputNames{"in"};
  static inline const Displayable::ControlNames m_ControlNames{"volume"};
  mutable Controllable::ControlValues m_ControlValues;
};

#endif