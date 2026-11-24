#ifndef NST_TEENSY_AUDIO_GRAPH_HPP_
#define NST_TEENSY_AUDIO_GRAPH_HPP_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>

#include "nst/inplace_vector.hpp"
#include "nst/strong_alias.hpp"

#include <Audio.h>

namespace nst::teensy {

struct AudioID : public nst::strong_alias<std::uint32_t, AudioID> {
	using nst::strong_alias<std::uint32_t, AudioID>::strong_alias;

	constexpr operator bool() const { return this->value != 0; }

	constexpr bool operator==(const AudioID &other) const {
		return this->value == other.value;
	}

	constexpr bool operator!=(const AudioID &other) const {
		return this->value != other.value;
	}
};

struct AudioNode {
	AudioID id;
	::AudioStream *device;
	AudioNode(AudioID id, ::AudioStream *device) : id{id}, device{device} {}
};

struct AudioPatch {
	AudioID src_id;
	std::uint8_t src_port;
	AudioID dst_id;
	std::uint8_t dst_port;
	::AudioConnection connection;

	AudioPatch(AudioNode src, std::uint8_t src_port, AudioNode dst,
	           std::uint8_t dst_port)
	    : src_id{src.id}, src_port{src_port}, dst_id{dst.id},
	      dst_port{dst_port},
	      connection{*src.device, src_port, *dst.device, dst_port} {}
};

// Fixed-size audio graph for the Teensy Audio Library. Holds and manages
// AudioConnections.
template <std::size_t NodeCapacity, std::size_t PatchCapacity>
class AudioGraph {
  public:
	[[nodiscard]] auto add(::AudioStream *device) -> std::optional<AudioID> {
		const auto new_id = node_id_gen_.next_id();
		if (!nodes_.emplace_back(new_id, device)) {
			return std::nullopt;
		}
		return new_id;
	}

	bool remove(AudioID id) {
		assert(id);
		auto it = find_node(id);
		if (it == nodes_.end()) {
			return false;
		}
		nodes_.erase(it);
		for (auto &p : patches_) {
			if (p && (p->src_id == id || p->dst_id == id)) {
				p.reset();
			}
		}
		return true;
	}

	bool connect(AudioID src_id, std::uint8_t src_port, AudioID dst_id,
	             std::uint8_t dst_port) {
		assert(src_id && dst_id);

		if (find_patch(src_id, src_port, dst_id, dst_port) != patches_.end()) {
			return false;
		}

		const auto src = find_node(src_id);
		const auto dst = find_node(dst_id);
		if (src == nodes_.end() || dst == nodes_.end()) {
			return false;
		}

		for (auto &p : patches_) {
			if (!p) {
				p.emplace(*src, src_port, *dst, dst_port);
				return true;
			}
		}
		return false;
	}

	// Connect two nodes on port 0
	bool connect(AudioID src_id, AudioID dst_id) {
		assert(src_id && dst_id);
		return connect(src_id, std::uint8_t{0}, dst_id, std::uint8_t{0});
	}

	bool disconnect(AudioID src_id, std::uint8_t src_port, AudioID dst_id,
	                std::uint8_t dst_port) {
		assert(src_id && dst_id);
		auto it = find_patch(src_id, src_port, dst_id, dst_port);
		if (it == patches_.end()) {
			return false;
		}
		it->reset();
		return true;
	}

	// Remove the first connection found between src and dst
	bool disconnect(AudioID src_id, AudioID dst_id) {
		assert(src_id && dst_id);
		for (auto &p : patches_) {
			if (p && p->src_id == src_id && p->dst_id == dst_id) {
				p.reset();
				return true;
			}
		}
		return false;
	}

	[[nodiscard]] float processor_usage() {
		float sum{};
		for (const auto n : nodes_) {
			sum += n.device->processorUsage();
		}
		return sum;
	}

	[[nodiscard]] float processor_usage_max() {
		float sum{};
		for (const auto n : nodes_) {
			sum += n.device->processorUsageMax();
		}
		return sum;
	}

	void processor_usage_max_reset() {
		for (const auto n : nodes_) {
			n.device->processorUsageMaxReset();
		}
	}

	[[nodiscard]] const auto &nodes() const { return nodes_; }
	[[nodiscard]] const auto &patches() const { return patches_; }

  private:
	auto find_node(AudioID id) {
		return std::find_if(nodes_.begin(), nodes_.end(),
		                    [id](const auto &n) { return n.id == id; });
	}

	auto find_node(AudioID id) const {
		return std::find_if(nodes_.cbegin(), nodes_.cend(),
		                    [id](const auto &n) { return n.id == id; });
	}

	auto find_patch(AudioID src_id, std::uint8_t src_port, AudioID dst_id,
	                std::uint8_t dst_port) const {
		return std::find_if(
		    patches_.cbegin(), patches_.cend(), [=](const auto &p) {
			    return p && src_id == p->src_id && src_port == p->src_port &&
			           dst_id == p->dst_id && dst_port == p->dst_port;
		    });
	}

	auto find_patch(AudioID src_id, AudioID dst_id) const {
		return std::find_if(
		    patches_.cbegin(), patches_.end(), [=](const auto &p) {
			    return p && src_id == p->src_id && dst_id == p->dst_id;
		    });
	}

	auto find_patch(AudioID src_id, std::uint8_t src_port, AudioID dst_id,
	                std::uint8_t dst_port) {
		return std::find_if(
		    patches_.begin(), patches_.end(), [=](const auto &p) {
			    return p && src_id == p->src_id && src_port == p->src_port &&
			           dst_id == p->dst_id && dst_port == p->dst_port;
		    });
	}

	struct {
		AudioID next_id() { return AudioID{++last_id_}; }
		AudioID::underlying_t last_id_{};
	} node_id_gen_;

	nst::inplace_vector<AudioNode, NodeCapacity> nodes_;
	std::array<std::optional<AudioPatch>, PatchCapacity> patches_;
};

} // namespace nst::teensy

#endif