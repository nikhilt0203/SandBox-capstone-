#ifndef mult_hpp_
#define mult_hpp_

#include "dep/module.hpp"
#include "dep/module_interfaces.hpp"
#include "dep/controls.hpp"
#include "audio/audio_pass_through.hpp"

class Mult 
: public Module,
  public Displayable
{
public:
  MODULE_TYPE_INFO("mult", "split 1 input to 5 outputs", 0x346575);

public:
  Mult() : Module(1, 5)
  {
    m_Audio.addDevice<AudioPassThrough>();
    auto device = m_Audio.device<AudioPassThrough>();
    m_Audio.mapInput(0, device, 0);
    for (std::size_t i{}; i < numOutputs(); i++) { m_Audio.mapOutput(i, device, 0); }
  }

  [[nodiscard]] std::string_view displayName() const override { return "mult"; }

  [[nodiscard]] const std::vector<std::string_view>& controlNames() const override 
  {
    static const std::vector<std::string_view> controlNames{};
    return controlNames;
  }

  [[nodiscard]] const std::vector<float>& normalizedControlValues() const override
  { 
    static std::vector<float> values{};
    return values;
  }

  [[nodiscard]] const std::vector<std::string_view>& inputNames() const override 
  {
    static const std::vector<std::string_view> inputNames{"in"};
    return inputNames;
  }

  [[nodiscard]] const std::vector<std::string_view>& outputNames() const override 
  {
    static const std::vector<std::string_view> outputNames{"1", "2", "3", "4", "5"};
    return outputNames;
  }

  [[nodiscard]] std::uint32_t displayColor() const override { return COLOR; }
};

#endif