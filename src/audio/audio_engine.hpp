#ifndef SANDBOX_AUDIO_ENGINE_HPP_
#define SANDBOX_AUDIO_ENGINE_HPP_

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <utility>

#include "audio/audio_trigger.hpp"
#include "config/config.hpp"
#include "modules/parameter.hpp"
#include <nst/audio_graph.hpp>

namespace sndbx {

using AudioGraph = nst::teensy::AudioGraph<limits::max_audio_graph_nodes,
                                           limits::max_audio_graph_patches>;

using AudioError = nst::teensy::AudioGraphError;

struct AudioEndpoint {
	nst::teensy::AudioNodeID id;
	nst::teensy::AudioPort port;
};

struct ModulePort {
	enum class Type : std::uint8_t { IN, OUT } type;
	std::uint8_t index;

	constexpr bool operator==(const ModulePort &other) const {
		return this->type == other.type && this->index == other.index;
	}

	constexpr bool operator!=(const ModulePort &other) const {
		return this->type != other.type || this->index != other.index;
	}
};

} // namespace sndbx

namespace sndbx::audio {

[[nodiscard]] inline auto make_endpoint(nst::teensy::AudioNodeID id,
                                        std::uint8_t graph_port) {
	return AudioEndpoint{id, nst::teensy::AudioPort{graph_port}};
}

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

// connect two Patchables by port index
inline AudioError connect(const Patchable &src, std::uint8_t output_idx,
                          const Patchable &dst, std::uint8_t input_idx,
                          AudioGraph &graph) {
	if (output_idx >= src.outs() || input_idx >= dst.ins()) {
		return AudioError::INVALID_PORT;
	}
	const auto src_endpt =
	    src.map(ModulePort{ModulePort::Type::OUT, output_idx});
	const auto dst_endpt =
	    dst.map(ModulePort{ModulePort::Type::IN, output_idx});
	return graph.connect(src_endpt.id, src_endpt.port, dst_endpt.id,
	                     dst_endpt.port);
}

// disconnect two Patchables by port index
inline AudioError disconnect(const Patchable &src, std::uint8_t output_idx,
                             const Patchable &dst, std::uint8_t input_idx,
                             AudioGraph &graph) {
	if (output_idx >= src.outs() || input_idx >= dst.ins()) {
		return AudioError::INVALID_PORT;
	}
	const auto src_endpt =
	    src.map(ModulePort{ModulePort::Type::OUT, output_idx});
	const auto dst_endpt =
	    dst.map(ModulePort{ModulePort::Type::IN, output_idx});
	return graph.disconnect(src_endpt.id, src_endpt.port, dst_endpt.id,
	                        dst_endpt.port);
}

// Query AudioGraph processor usage
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

} // namespace sndbx::audio

#endif