#ifndef SANDBOX_MULT_HPP_
#define SANDBOX_MULT_HPP_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "audio/audio_pass_through.hpp"

class Mult 
  : public Module,
    public Displayable
{
public:
  MODULE_TYPE_INFO("mult", "split 1 input to 8 outputs", 0x346575);

public:
  Mult() : Module(1, 8)
  {
    m_Audio.addDevice<AudioPassThrough>();
    const auto device = m_Audio.device<AudioPassThrough>();
    m_Audio.mapInput(0, device, 0);
    for (std::size_t i{}; i < numOutputs(); i++) { m_Audio.mapOutput(i, device, 0); }
  }

  [[nodiscard]] std::string_view displayName() const override { return NAME; }
  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }

  [[nodiscard]] auto controlNames() const 
    -> const std::vector<std::string_view>& override { return emptyVectorSV(); }

  [[nodiscard]] auto normalizedControlValues() const 
    -> const std::vector<float>& override { return emptyVectorFloat(); }

  [[nodiscard]] auto inputNames() const 
    -> const std::vector<std::string_view>& override { return m_InputNames; }

  [[nodiscard]] auto outputNames() const 
    -> const std::vector<std::string_view>& override { return m_OutputNames; }

private:
  inline static const std::vector<std::string_view> m_InputNames{"in"};
  inline static const std::vector<std::string_view> m_OutputNames{"1", "2", "3", "4", "5", "6", "7", "8"};
  mutable std::vector<float> m_ControlValues{};
};

#endif