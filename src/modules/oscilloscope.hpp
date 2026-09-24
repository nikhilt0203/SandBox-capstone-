#ifndef SANDBOX_OSCILLOSCOPE_HPP_
#define SANDBOX_OSCILLOSCOPE_HPP_

#include "audio/audio_shared_buffer.hpp"
#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "ui/screen_elements.hpp"

class Oscilloscope final : public Module,
                           public Displayable,
                           public Animatable {
  static constexpr std::size_t bufferSize = 1024;
  using AudioBuffer = AudioBufferShared<bufferSize>;

public:
  MODULE_TYPE_INFO("scope", "displays the input signal", 0x34FF75);
  constexpr static auto max_count = 8U;

public:
  Oscilloscope() : Module(1, 1) {
    m_Audio.addDevice<AudioBuffer>();
    m_SharedBuffer = m_Audio.device<AudioBuffer>();
    m_Audio.mapInput(0, m_SharedBuffer, 0);
    m_Audio.mapOutput(0, m_SharedBuffer, 0);
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

  void drawNext(GFXcanvas16 &frame) const override {
    // Only one oscilloscope has permission to write to the buffer at a single
    // time
    m_SharedBuffer->setBufferWriteResponsibility();
    if (m_SharedBuffer->isFull()) {
      WaveformDisplayFrame{m_SharedBuffer->flush(), frame}.draw();
    }
  }

private:
  AudioBuffer *m_SharedBuffer{};

  static inline const Displayable::PortNames m_InputNames{"in"};
  static inline const Displayable::PortNames m_OutputNames{"out"};
};

#endif