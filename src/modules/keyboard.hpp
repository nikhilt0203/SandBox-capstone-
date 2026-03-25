#ifndef SANDBOX_KEYBOARD_HPP_
#define SANDBOX_KEYBOARD_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/parameter.hpp"
#include "audio/audio_trigger_output.hpp"

class Keyboard 
  : public Module, 
    public Controllable,
    public Displayable
{
public:
  MODULE_TYPE_INFO("keyboard", "", 0x367591);

  enum class Scale
  {
    Major,
    Minor,
    MajorPentatonic,
    MinorPentatonic
  };

  using LengthChangeCallback = bool(*)(Keyboard&);

  using KeyboardID = std::uint32_t;
  using ScaleChangeCallback = void(*)(Scale, KeyboardID);

public:
  Keyboard();

  void changeControl(std::size_t index, int delta) override;
  void resetControls() override;

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;

  [[nodiscard]] std::size_t numKeys() const { return m_NumKeys; }
  [[nodiscard]] Scale scale();

  void onKeyPress(float amplitude);
  void onKeyRelease();

  void setAddKeyCallback(LengthChangeCallback cb) { m_AddKeyCallback = cb; }
  void setSubtractKeyCallback(LengthChangeCallback cb) { m_SubtractKeyCallback = cb; }
  void setScaleChangeCallback(ScaleChangeCallback cb) { m_ScaleChangeCallback = cb; }

private:
  void lengthAdjust(int delta);
  void scaleAdjust(int delta);

private:
  ModuleParameter<std::size_t> m_NumKeys{0U, 0U, 32U};
  ModuleParameter<std::uint8_t> m_Scale{0U, 0U, 4U};

  LengthChangeCallback m_AddKeyCallback;
  LengthChangeCallback m_SubtractKeyCallback;
  ScaleChangeCallback m_ScaleChangeCallback;

  AudioSynthWaveformDc* m_DC{nullptr};
  AudioTriggerOutput* m_TrigOut{nullptr};

  std::size_t m_NumKeysOn{};

  inline static sndbx::vector_8U<std::string_view> m_OutputNames{"cv", "trg"};
  inline static sndbx::vector_4U<std::string_view> m_ControlNames{"length", "scale"};
  mutable sndbx::vector_4U<float> m_ControlValues{};
};


class KeyboardKey
  : public Module, 
    public Displayable,
    public Pressable,
    public Controllable
{
public:
  MODULE_TYPE_INFO("key", "", 0x2222FF);

public:
  KeyboardKey() : Module(0, 0) {}

  void changeControl(std::size_t index, int delta) override { if (index == 0) { adjustAmplitude(delta); } }
  void resetControls() override { m_Amplitude.reset(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }

  [[nodiscard]] auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto normalizedControlValues() const -> const sndbx::vector_4U<float>& override;

  [[nodiscard]] float amplitude() const { return m_Amplitude; }

  void setAmplitude(float amplitude);
  void setParent(Keyboard* parent) { m_Parent = parent; }

  void onRisingEdge() { m_Parent->onKeyPress(m_Amplitude); }
  void onFallingEdge() { m_Parent->onKeyRelease(); }

private:
  void adjustAmplitude(int delta);
  void updateColor();

private:
  ModuleParameter<float> m_Amplitude{0.0f, 0.0f, 1.0f};

  Keyboard* m_Parent{};

  std::uint32_t m_LEDColor{COLOR};

  inline static sndbx::vector_4U<std::string_view> m_ControlNames{"value"};
  mutable sndbx::vector_4U<float> m_ControlValues{};
};

#endif