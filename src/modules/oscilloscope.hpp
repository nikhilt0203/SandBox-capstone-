#ifndef SANDBOX_OSCILLOSCOPE_HPP_
#define SANDBOX_OSCILLOSCOPE_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "audio/audio_buffer_input.hpp"
#include "ui/screen_elements.hpp"

class Oscilloscope 
: public Module,
  public Displayable,
  public Animatable
{
static constexpr std::size_t bufferSize = 1024;
using AudioBuffer = AudioBufferInput<bufferSize>;

public:
  MODULE_TYPE_INFO("scope", "displays the input signal", 0x34FF75);
  
public:
  Oscilloscope() : Module(1, 1)
  {
    m_Audio.addDevice<AudioBuffer>();
    m_AudioBuffer = m_Audio.device<AudioBuffer>();
    m_Audio.mapInput(0, m_AudioBuffer, 0);
    m_Audio.mapOutput(0, m_AudioBuffer, 0); 
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto inputNames() const 
    -> const std::vector<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const 
    -> const std::vector<std::string_view>& override { return m_OutputNames; }

  [[nodiscard]] auto controlNames() const 
    -> const std::vector<std::string_view>& override { return emptyVectorSV(); }

  [[nodiscard]] auto normalizedControlValues() const 
    -> const std::vector<float>& override { return emptyVectorFloat(); }

  void drawNext(GFXcanvas16& frame) const override { WaveformDisplayFrame{m_AudioBuffer->flush(), frame}.draw(); }

private: 
  AudioBuffer* m_AudioBuffer{};

  inline static const std::vector<std::string_view> m_InputNames{"in"};
  inline static const std::vector<std::string_view> m_OutputNames{"out"};
};

#endif