#ifndef SANDBOX_KEYBOARD_HPP_
#define SANDBOX_KEYBOARD_HPP_

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
public:
  MODULE_TYPE_INFO("keyboard", "", 0x367591);

  enum struct Scale
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

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto controlNames() const 
    -> const std::vector<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto outputNames() const 
    -> const std::vector<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto inputNames() const 
    -> const std::vector<std::string_view>& override { return emptyVectorSV(); }

  [[nodiscard]] auto normalizedControlValues() const -> const std::vector<float>& override;

  [[nodiscard]] std::size_t numKeys() const { return m_NumKeys; }
  [[nodiscard]] Scale scale();

  void keyPress(float amplitude);
  void keyRelease();

  void setAddKeyCallback(LengthChangeCallback cb) { m_AddKeyCallback = cb; }
  void setSubtractKeyCallback(LengthChangeCallback cb) { m_SubtractKeyCallback = cb; }
  void setScaleChangeCallback(ScaleChangeCallback cb) { m_ScaleChangeCallback = cb; }

private:
  void lengthAdjust(int delta);
  void scaleAdjust(int delta);

private:
  Parameter<std::size_t> m_NumKeys{0U, 0U, 32U};
  Parameter<std::uint8_t> m_Scale{0U, 0U, 4U};

  Controls m_Controls{ 
    [this](int delta){ lengthAdjust(delta); },
    [this](int delta){ scaleAdjust(delta); }
  };

  LengthChangeCallback m_AddKeyCallback;
  LengthChangeCallback m_SubtractKeyCallback;
  ScaleChangeCallback m_ScaleChangeCallback;

  AudioSynthWaveformDc* m_DC{nullptr};
  AudioTriggerOutput* m_TrigOut{nullptr};

  std::size_t m_NumKeysOn{};

  inline static std::vector<std::string_view> m_OutputNames{"cv", "trg"};
  inline static std::vector<std::string_view> m_ControlNames{"length", "scale"};
  mutable std::vector<float> m_ControlValues{};
};


class KeyboardKey
  : public Module, 
    public Displayable,
    public Pressable,
    public Controllable
{
public:
  MODULE_TYPE_INFO("key", "", 0x3675DE);

public:
  KeyboardKey() 
  : Module(0, 0)
  {}

  void changeControl(std::size_t index, int delta) override { m_Controls.change(index, delta); }
  [[nodiscard]] std::size_t numControls() const override { return m_Controls.size(); }

  [[nodiscard]] std::string_view displayName() const override { return NAME; };
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
  [[nodiscard]] std::uint32_t ledColor() const override { return m_LEDColor; }

  [[nodiscard]] auto controlNames() const 
    -> const std::vector<std::string_view>& override { return m_ControlNames; }

  [[nodiscard]] auto inputNames() const 
    -> const std::vector<std::string_view>& override { return emptyVectorSV(); }

  [[nodiscard]] auto outputNames() const 
    -> const std::vector<std::string_view>& override { return emptyVectorSV(); }

  [[nodiscard]] auto normalizedControlValues() const -> const std::vector<float>& override;

  [[nodiscard]] float amplitude() const { return m_Amplitude; }
  void setAmplitude(float amplitude);

  void setParent(Keyboard* parent) { m_Parent = parent; }

  void onRisingEdge() { m_Parent->keyPress(m_Amplitude); }
  void onFallingEdge() { m_Parent->keyRelease(); }

private:
  void adjustAmplitude(int delta);
  void updateColor();

private:
  Parameter<float> m_Amplitude{0.0f, 0.0f, 1.0f};

  Controls m_Controls{ [this](int delta){ adjustAmplitude(delta); } };

  Keyboard* m_Parent{};

  std::uint32_t m_LEDColor{COLOR};

  inline static std::vector<std::string_view> m_ControlNames{"value"};
  mutable std::vector<float> m_ControlValues{};
};

#endif