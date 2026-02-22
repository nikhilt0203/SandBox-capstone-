#ifndef oscilloscope_hpp_
#define oscilloscope_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "audio/audio_buffer_input.hpp"
#include "ui/screen_elements.hpp"

class Oscilloscope 
: public Module,
  public Animatable
{
using AudioBuffer = AudioBufferInput<1024>;

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

  void drawNext(GFXcanvas16& frame) const override
  {
    WaveformDisplayFrame{m_AudioBuffer->flush(), frame}.draw();
  }

private: 
  AudioBuffer* m_AudioBuffer{};
};

#endif