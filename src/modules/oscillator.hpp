#ifndef SANDBOX_OSCILLATOR_HPP_
#define SANDBOX_OSCILLATOR_HPP_

#include "audio/audio_engine.hpp"
#include "controllable.hpp"
#include "displayable.hpp"
#include "parameter.hpp"

namespace sndbx {

struct Waveform {
  constexpr Waveform(short id, std::string_view name, std::uint32_t color)
      : id{id}, name{name}, color{color} {}

  short id;
  std::string_view name;
  std::uint32_t color;
};

class Oscillator : public audio::Patchable,
                   public Controllable,
                   public Displayable {
public:
  constexpr static std::string_view name = "oscillator";
  constexpr static std::string_view desc = "outputs a continuous waveform";

  Oscillator() : Patchable{2, 1} {
    frequency_ = 440.0f;
    fine_tune_ = 0;
    fm_depth_ = 8.25f;
    waveform_ = 0;
  }

  void change(std::uint8_t ctrl, std::int8_t amt) override {
    switch (ctrl) {
    case 0:
      coarse_tune(amt);
      break;
    case 1:
      fine_tune(amt);
      break;
    case 2:
      fm_adjust(amt);
      break;
    case 3:
      change_waveform(amt);
      break;
    }
  }

  auto link(AudioGraph &graph) -> AudioError override {
    auto id = graph.add_node(&synth_);
    if (!id) {
      return id.error();
    }
    synth_id_ = *id;
    return AudioError::NONE;
  }

  auto map(ModulePort port) const -> AudioEndpoint override {
    return audio::make_endpoint(synth_id_, 0);
  }

  void unlink(AudioGraph &graph) override { graph.remove_node(synth_id_); }

  std::string_view display_name() const override {
    return waveforms[waveform_].name;
  }

  std::uint16_t display_color() const override {
    return waveforms[waveform_].color;
  }

private:
  void set_waveform(std::size_t waveform) {
    waveform_ = waveform;
    synth_.begin(waveforms[waveform_].id);
  }

  void coarse_tune(int amt) {
    frequency_ *= powf(1.08f, amt);
    synth_.frequency(frequency_ + fine_tune_);
  }

  void fine_tune(int amt) {
    fine_tune_ += amt;
    synth_.frequency(frequency_ + fine_tune_);
  }

  void fm_adjust(int amt) {
    fm_depth_ += 0.25f * amt;
    synth_.frequencyModulation(fm_depth_);
  }

  void change_waveform(int amt) {
    int total = num_waveforms;
    const auto wrapped_idx =
        ((static_cast<int>(waveform_) + amt) % total + total) % total;
    set_waveform(wrapped_idx);
  }

  void init_synth() {
    synth_.frequency(frequency_);
    synth_.frequencyModulation(fm_depth_);
    set_waveform(waveform_);
  }

protected:
  AudioSynthWaveformModulated synth_;
  nst::teensy::AudioNodeID synth_id_;

  ModuleParameter<float> frequency_{0.01f, 18000.0f};
  ModuleParameter<int> fine_tune_{-50, 50};
  ModuleParameter<float> fm_depth_{0.0f, 12.0f};
  ModuleParameter<std::size_t> waveform_{0U, num_waveforms - 1};

  static constexpr std::size_t num_waveforms = 7U;
  static constexpr std::array<Waveform, num_waveforms> waveforms = {
      Waveform{WAVEFORM_SINE, "sine", 0x0FF00},
      Waveform{WAVEFORM_SQUARE, "square", 0xFF000},
      Waveform{WAVEFORM_SAWTOOTH, "saw", 0xFF00F},
      Waveform{WAVEFORM_TRIANGLE, "triangle", 0xFFF00},
      Waveform{WAVEFORM_PULSE, "pulse", 0x00832},
      Waveform{WAVEFORM_SAWTOOTH_REVERSE, "rev saw", 0xB817E},
      Waveform{WAVEFORM_SAMPLE_HOLD, "s&h noise", 0x09F77}};
};

} // namespace sndbx

#endif