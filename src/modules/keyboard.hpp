#ifndef keyboard_hpp_
#define keyboard_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_trigger_output.hpp"

class Keyboard 
  : public Module, 
    public Controllable,
    public Displayable
{
  using LengthChangeCallback = bool(*)(Keyboard&);
public:
  MODULE_TYPE_INFO("keyboard", "", 0x367591);

  Keyboard();

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override { return m_ControlNames; }
  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override { return m_OutputNames; }
  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override { return EMPTY; }
  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override;

  [[nodiscard]] std::size_t numKeys() const { return m_NumKeys; }

  void on(float amplitude);
  void off() { m_TrigOut->off(); }

  void setAddKeyCallback(LengthChangeCallback cb) { m_AddKeyCallback = cb; }
  void setSubtractKeyCallback(LengthChangeCallback cb) { m_SubtractKeyCallback = cb; }

private:
  void lengthAdjust(int delta);

protected:
  Parameter<std::size_t> m_NumKeys{0U, 0U, 32U};
  Parameter<std::size_t> m_Scale{0U, 0U, 5U};

  AudioSynthWaveformDc* m_DC{nullptr};
  AudioTriggerOutput* m_TrigOut{nullptr};

  LengthChangeCallback m_AddKeyCallback;
  LengthChangeCallback m_SubtractKeyCallback;

  Controls m_Controls{ [this](int delta){ lengthAdjust(delta); } };

  inline static std::vector<std::string_view> m_OutputNames{"cv", "trg"};
  inline static std::vector<std::string_view> m_ControlNames{"length"};
  mutable std::vector<float> m_ControlValues{};
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
  KeyboardKey(Keyboard& parent) : Module(0, 0), m_Parent(parent) {}

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override { return m_ControlNames; }
  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override;
  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override { return EMPTY; }
  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override { return EMPTY; }

  void setAmplitude(float amplitude) { m_Amplitude.value() = std::clamp(amplitude, m_Amplitude.min(), m_Amplitude.max()); }
  [[nodiscard]] std::uint32_t parentID() { return m_Parent.id(); }

  void onRisingEdge() { m_Parent.on(m_Amplitude); }
  void onFallingEdge() { m_Parent.off(); }

private:
  void adjustAmplitude(int delta);

private:
  Parameter<float> m_Amplitude{0.0f, 0.0f, 1.0f};

  Controls m_Controls{ [this](int delta){ adjustAmplitude(delta); } };
  Keyboard& m_Parent;

  inline static std::vector<std::string_view> m_ControlNames{"value"};
  mutable std::vector<float> m_ControlValues{};
};

#endif