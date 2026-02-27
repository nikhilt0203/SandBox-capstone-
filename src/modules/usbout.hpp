#ifndef SANDBOX_USBOUT_HPP_
#define SANDBOX_USBOUT_HPP_

#include "dep/module.hpp"
#include "dep/controls.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"
#include "audio/output_i2smixer.hpp"
#include "audio/output_usbmixer.hpp"

//===================================================
// USBOUT
//===================================================
class I2SOut 
: public Module,
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("audio out", "outputs audio via speakers", 0x0000FF);

public:
  I2SOut() 
  : Module(1, 0)
  {
    enableAudioShield();
    m_Audio.addDevice<AudioOutputI2SMixer>();
    m_OutputMixer = m_Audio.device<AudioOutputI2SMixer>();
    m_Audio.mapInput(0, m_OutputMixer, 0);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; };

  [[nodiscard]] auto controlNames() const -> const std::vector<std::string_view>& override 
  {
    static const std::vector<std::string_view> controlNames{"volume"};
    return controlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const -> const std::vector<float>& override
  { 
    static std::vector<float> values;
    values.clear();
    values.push_back(m_Volume.normalized());
    return values;
  }

  [[nodiscard]] auto inputNames() const -> const std::vector<std::string_view>& override 
  {
    static const std::vector<std::string_view> inputNames{"in"};
    return inputNames;
  }

  [[nodiscard]] auto outputNames() const -> const std::vector<std::string_view>& override 
  {
    return emptyVectorSV();
  }

private:
  void volumeAdjust(int delta) 
  { 
    auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1*d); };
    m_Volume.change(gainCurve, delta); 
    m_OutputMixer->volume(m_Volume);
  }

private:
  Parameter<float> m_Volume{0.5f, 0.0f, 2.0f};

  Controls m_Controls{ [this](int delta){ volumeAdjust(delta); }};

  AudioOutputI2SMixer* m_OutputMixer;

private:
  inline static AudioControlSGTL5000 m_AudioShield{};

  static void enableAudioShield()
  {
    static bool enabled = false;
    if (enabled) { return; }
    m_AudioShield.enable();
    m_AudioShield.volume(0.5);
    enabled = true;
  }
};

// USB OUT
class USBOut 
: public Module,
  public Controllable,
  public Displayable
{
public:
  MODULE_TYPE_INFO("audio out", "outputs audio via usb or speakers", 0x0000FF);

public:
  USBOut() : Module(1, 0)
  {
    m_Audio.addDevice<AudioOutputUSBMixer>();
    m_OutputMixer = m_Audio.device<AudioOutputUSBMixer>();
    m_Audio.mapInput(0, m_OutputMixer, 0);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; };

  [[nodiscard]] auto controlNames() const -> const std::vector<std::string_view>& override 
  {
    static const std::vector<std::string_view> controlNames{"volume"};
    return controlNames;
  }

  [[nodiscard]] auto normalizedControlValues() const -> const std::vector<float>& override
  { 
    static std::vector<float> values;
    values.clear();
    values.push_back(m_Volume.normalized());
    return values;
  }

  [[nodiscard]] auto inputNames() const -> const std::vector<std::string_view>& override 
  {
    static const std::vector<std::string_view> inputNames{"in"};
    return inputNames;
  }

  [[nodiscard]] auto outputNames() const -> const std::vector<std::string_view>& override 
  {
    return emptyVectorSV();
  }

private:
  void volumeAdjust(int delta) 
  { 
    auto gainCurve = [](float v, int d) { return v * powf(1.10f, 1*d); };
    m_Volume.change(gainCurve, delta); 
    m_OutputMixer->volume(m_Volume);
  }

private:
  Parameter<float> m_Volume{1.0f, 0.0f, 2.0f};

  Controls m_Controls{ [this](int delta){ volumeAdjust(delta); }};

  AudioOutputUSBMixer* m_OutputMixer{};
};

#endif