#pragma once

#include "audio/audio_trigger.hpp"
#include "audio_config.hpp"
#include "audio_graph.hpp"
#include "module_parameter.hpp"
#include <algorithm>
#include <utility>

namespace sndbx {

// Configure audio graph
using AudioGraph = nst::teensy::AudioGraph<config::audio::max_modules,
                                           config::audio::max_connections>;
using AudioError = AudioGraph::Error;

template <std::size_t N>
using AudioNodeIDs = nst::inplace_vector<AudioGraph::NodeID, N>;

struct AudioEndpoint {
  AudioGraph::NodeID id;
  AudioGraph::Port port;
};

struct ModulePort : public nst::strong_alias<std::uint8_t, ModulePort> {
  using nst::strong_alias<std::uint8_t, ModulePort>::strong_alias;
  enum class Type : std::uint8_t { IN, OUT } type;

  constexpr bool operator==(const ModulePort &other) const {
    return this->type == other.type && this->value == other.value;
  }

  constexpr bool operator!=(const ModulePort &other) const {
    return this->type != other.type || this->value != other.value;
  }
};

// Interface between module and audio graph.
class Patchable {
public:
  Patchable(std::uint8_t ins, std::uint8_t outs) : ins_{ins}, outs_{outs} {}
  virtual ~Patchable() = default;

  [[nodiscard]] virtual auto link(AudioGraph &graph) -> AudioError = 0;
  virtual void unlink(AudioGraph &graph) = 0;
  [[nodiscard]] virtual auto map(ModulePort port) const -> AudioEndpoint = 0;

  [[nodiscard]] std::uint8_t ins() const { return ins_; }
  [[nodiscard]] std::uint8_t outs() const { return outs_; }

private:
  std::uint8_t ins_;
  std::uint8_t outs_;
};

} // namespace sndbx

namespace sndbx::audio {

namespace impl {
template <class AudioStream, typename IDContainer>
bool try_emplace(AudioGraph &graph, IDContainer &ids, AudioError &out_error) {
  if (out_error != AudioError::NONE) {
    return false;
  }
  if (auto result = graph.make_node<AudioStream>(); !result) {
    out_error = result.error();
    return false;
  } else {
    ids.emplace_back(result.value());
    return true;
  }
}

} // namespace impl

template <typename AudioDevice, std::size_t Max> inline auto &pool() {
  static_assert(std::is_base_of_v<::AudioStream, AudioDevice>);
  static nst::object_pool<AudioDevice, Max> pool;
  return pool;
}

template <class... AudioStreams, typename IDContainer>
inline auto emplace_in_graph(AudioGraph &graph, IDContainer &ids_out) {
  AudioError out_error{AudioError::NONE};
  // short circuits if error occurs
  (impl::try_emplace<AudioStreams>(graph, ids_out, out_error) && ...);
  return out_error;
}

template <typename AudioDevice, std::size_t N>
auto add_from_pool(AudioGraph &graph, nst::object_pool<AudioDevice, N> &pool)
    -> nst::expected<AudioGraph::Receipt<AudioDevice>, AudioError> {
  if (auto s = pool.acquire(); !s) {
    return AudioError::POOL_EXHAUSTED;
  } else if (auto id = graph.add_node(s); !id) {
    return id.error();
  } else {
    return AudioGraph::Receipt<AudioDevice>{*id, s};
  }
}

inline auto connect(const Patchable &src, ModulePort src_port,
                    const Patchable &dst, ModulePort dst_port,
                    AudioGraph &graph) {
  const auto src_endpt = src.map(src_port);
  const auto dst_endpt = dst.map(dst_port);
  return graph.connect(src_endpt.id, src_endpt.port, dst_endpt.id,
                       dst_endpt.port);
}

inline auto disconnect(const Patchable &src, ModulePort src_port,
                       const Patchable &dst, ModulePort dst_port,
                       AudioGraph &graph) {
  const auto src_endpt = src.map(src_port);
  const auto dst_endpt = dst.map(dst_port);
  return graph.disconnect(src_endpt.id, src_endpt.port, dst_endpt.id,
                          dst_endpt.port);
}

inline auto connect(AudioEndpoint src, AudioEndpoint dst, AudioGraph &graph) {
  return graph.connect(src.id, src.port, dst.id, dst.port);
}

inline auto disconnect(AudioEndpoint src, AudioEndpoint dst,
                       AudioGraph &graph) {
  return graph.disconnect(src.id, src.port, dst.id, dst.port);
}

[[nodiscard]] inline float processor_usage(AudioGraph &graph) {
  float sum{};
  for (const auto &n : graph.nodes()) {
    sum += n.device->processorUsage();
  }
  return sum;
}

[[nodiscard]] inline float processor_usage_max(AudioGraph &graph) {
  float sum{};
  for (const auto &n : graph.nodes()) {
    sum += n.device->processorUsageMax();
  }
  return sum;
}

inline void processor_usage_max_reset(AudioGraph &graph) {
  for (const auto &n : graph.nodes()) {
    n.device->processorUsageMaxReset();
  }
}

class IDisplayable {
public:
  virtual ~IDisplayable() = default;
  virtual std::string_view name() const = 0;
};

class IControllable {
public:
  virtual ~IControllable() = default;
  virtual void change(std::size_t p, int d) = 0;
  virtual void reset() = 0;
};

struct Waveform {
  constexpr Waveform(short id, std::string_view name, std::uint32_t color)
      : id{id}, name{name}, color{color} {}

  short id;
  std::string_view name;
  std::uint32_t color;
};

class Oscillator : public Patchable, public IControllable, public IDisplayable {
public:
  using Synth = AudioSynthWaveformModulated;
  constexpr static auto max_count = 32U;

  Oscillator() : Patchable(2, 1) { set_defaults(); }

  void change(std::size_t index, int amt) override {
    switch (index) {
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

  void reset() override { set_defaults(); }

  void set_defaults() {
    frequency_ = 440.0f;
    fine_tune_ = 0;
    fm_depth_ = 8.25f;
    waveform_ = 0;
  }

  auto link(AudioGraph &graph) -> AudioError override {
    auto r = audio::add_from_pool(graph, synth_pool_);
    if (!r) {
      return r.error();
    }
    synth_ = *r;
  }

  void unlink(AudioGraph &graph) override {
    graph.remove(synth_.id);
    synth_pool_.release(synth_.device);
  }

  [[nodiscard]] std::string_view name() const override {
    return waveforms[waveform_].name;
  }

private:
  void set_waveform(std::size_t waveform) {
    waveform_ = waveform;
    synth_->begin(waveforms[waveform_].id);
  }

  void coarse_tune(int amt) {
    frequency_ *= powf(1.08f, amt);
    synth_->frequency(frequency_ + fine_tune_);
  }

  void fine_tune(int amt) {
    fine_tune_ += amt;
    synth_->frequency(frequency_ + fine_tune_);
  }

  void fm_adjust(int amt) {
    fm_depth_ += 0.25f * amt;
    synth_->frequencyModulation(fm_depth_);
  }

  void change_waveform(int amt) {
    int total = num_waveforms;
    const auto new_idx =
        ((static_cast<int>(waveform_) + amt) % total + total) % total;
    set_waveform(new_idx);
  }

  void init_synth() {
    synth_->frequency(frequency_);
    synth_->frequencyModulation(fm_depth_);
    set_waveform(waveform_);
  }

protected:
  AudioGraph::Receipt<Synth> synth_{};

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

  static nst::object_pool<Synth, max_count> synth_pool_;
};

} // namespace sndbx::audio