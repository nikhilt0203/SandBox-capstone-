#ifndef keyboard_hpp_
#define keyboard_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_trigger_output.hpp"
#include "grid.hpp"

class Keyboard 
  : public Module, 
    public Controllable,
    public Displayable
{
  using LengthChangeCallback = bool(*)(Keyboard&);
public:
  MODULE_TYPE_INFO("keyboard", "", 0x367591);

  Keyboard() : Module(0, 1) 
  {
    m_Audio.addDevice<AudioSynthWaveformDc>();
    m_Audio.addDevice<AudioTriggerOutput>();

    m_DC = m_Audio.device<AudioSynthWaveformDc>();
    m_TrigOut = m_Audio.device<AudioTriggerOutput>(1);
    m_Audio.mapOutput(0, m_DC, 0);
    m_Audio.mapOutput(1, m_TrigOut, 0);
  }

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override
  {
    static std::vector<std::string_view> controlNames{"length"};
    return controlNames;
  }

  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override
  {
    static std::vector<float> values{};
    values.clear();
    values.reserve(2);
    values.push_back(m_NumKeys);
    values.push_back(m_Scale);
    return values;
  }

  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override
  {
    static std::vector<std::string_view> inputNames{};
    return inputNames;
  }

  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override
  {
    static std::vector<std::string_view> outputNames{"cv", "trg"};
    return outputNames;
  }

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  void on(float amplitude) { m_DC->amplitude(amplitude); m_TrigOut->on(); }
  void off() { m_TrigOut->off(); }

  void setAddKeyCallback(LengthChangeCallback cb) { m_AddKeyCallback = cb; }
  void setSubtractKeyCallback(LengthChangeCallback cb) { m_SubtractKeyCallback = cb; }

  [[nodiscard]] std::size_t numKeys() const { return m_NumKeys; }

private:
  void lengthAdjust(int delta)
  {
    auto lenCurve = [](std::size_t cur, int delta){ return cur + 1*delta; };

    if (!m_AddKeyCallback || !m_SubtractKeyCallback) { return; }

    bool success = delta > 0 ? m_AddKeyCallback(*this) 
                             : m_SubtractKeyCallback(*this);

    if (success) { m_NumKeys.change(lenCurve, delta); }
  }

protected:
  Parameter<std::size_t> m_NumKeys{8U, 0U, 32U};
  Parameter<std::size_t> m_Scale{0U, 0U, 5U};

  AudioSynthWaveformDc* m_DC{nullptr};
  AudioTriggerOutput* m_TrigOut{nullptr};

  LengthChangeCallback m_AddKeyCallback;
  LengthChangeCallback m_SubtractKeyCallback;

  Controls m_Controls{ [this](int delta){ lengthAdjust(delta); } };
};


class KeyboardKey
  : public Module, 
    public Displayable,
    public Pressable,
    public Controllable
{
public:
  MODULE_TYPE_INFO("key", "", 0x367591);

public:
  KeyboardKey(Keyboard& parent) 
  : Module(0, 0),
    m_Parent(parent) 
  {}

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  std::string_view displayName() const override { return NAME; };

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override
  {
    static std::vector<std::string_view> controlNames{"value"};
    return controlNames;
  }

  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override
  {
    static std::vector<float> values{};
    values.clear();
    values.push_back(m_Amplitude);
    return values;
  }

  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override
  {
    static std::vector<std::string_view> inputNames{};
    return inputNames;
  }

  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override
  {
    static std::vector<std::string_view> outputNames{};
    return outputNames;
  }

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  void setAmplitude(float amplitude) { m_Amplitude.value() = std::clamp(amplitude, m_Amplitude.min(), m_Amplitude.max()); }
  [[nodiscard]] std::uint32_t parentID() { return m_Parent.id(); }

  void onRisingEdge() { m_Parent.on(m_Amplitude); }
  void onFallingEdge() { m_Parent.off(); }

private:
  void adjustAmplitude(int delta)
  {
    auto amplitudeCurve = [](float cur, int delta){ return cur + 0.05f*delta; };
    m_Amplitude.change(amplitudeCurve, delta);
  }

private:
  Parameter<float> m_Amplitude{0.0f, 0.0f, 1.0f};

  Controls m_Controls{ [this](int delta){ adjustAmplitude(delta); } };
  Keyboard& m_Parent;
};

#endif