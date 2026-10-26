#ifndef NST_TEENSY_AUDIO_GRAPH_HPP_
#define NST_TEENSY_AUDIO_GRAPH_HPP_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>

#include "nst/expected.hpp"
#include "nst/inplace_vector.hpp"
#include "nst/object_pool.hpp"
#include "nst/strong_alias.hpp"

#include <Audio.h>

namespace nst::teensy {

struct AudioPort : public nst::strong_alias<std::uint8_t, AudioPort> {
	using nst::strong_alias<std::uint8_t, AudioPort>::strong_alias;

	constexpr bool operator==(const AudioPort &other) const {
		return this->value == other.value;
	}

	constexpr bool operator!=(const AudioPort &other) const {
		return this->value != other.value;
	}

	[[nodiscard]] constexpr auto operator*() const { return this->value; }
};

struct AudioNodeID : public nst::strong_alias<std::uint32_t, AudioNodeID> {
	using nst::strong_alias<std::uint32_t, AudioNodeID>::strong_alias;

	constexpr operator bool() const { return this->value != 0; }

	constexpr bool operator==(const AudioNodeID &other) const {
		return this->value == other.value;
	}

	constexpr bool operator!=(const AudioNodeID &other) const {
		return this->value != other.value;
	}
};

template <class AudioDevice> struct AudioNodeHandle {
	AudioNodeID id;
	AudioDevice *device;

	const AudioDevice *operator->() const { return device; }
	AudioDevice *operator->() { return device; }
	operator AudioNodeID() const { return id; }
};

struct AudioNode {
	AudioNodeID id;
	::AudioStream *device;
	AudioNode(AudioNodeID id, ::AudioStream *device) : id{id}, device{device} {}
};

struct AudioPatch {
	AudioNodeID src_id;
	AudioPort src_port;
	AudioNodeID dst_id;
	AudioPort dst_port;
	::AudioConnection connection;

	AudioPatch(AudioNode src, AudioPort src_port, AudioNode dst,
	           AudioPort dst_port)
	    : src_id{src.id}, src_port{src_port}, dst_id{dst.id},
	      dst_port{dst_port},
	      connection{*src.device, *src_port, *dst.device, *dst_port} {}
};

enum class AudioGraphError {
	NONE,
	PATCHES_EXHAUSTED,
	PATCH_ALREADY_EXISTS,
	NODE_NOT_FOUND,
	PATCH_NOT_FOUND,
	GRAPH_FULL,
	INVALID_PORT
};

// Fixed-size audio graph for Teensy Audio Library. Abstracts AudioStreams
// and AudioConnections into ID-based graph.
template <std::size_t NodeCapacity, std::size_t PatchCapacity>
class AudioGraph {
  public:
	using Error = AudioGraphError;

	auto add(::AudioStream *device) -> nst::expected<AudioNodeID, Error> {
		if (nodes_.is_full()) {
			return Error::GRAPH_FULL;
		}
		return emplace_and_get_id(device);
	}

	bool remove(AudioNodeID id) {
		auto it = find_node(id);
		if (it == nodes_.end()) {
			return false;
		}
		nodes_.erase(it);
		patches_.erase(std::remove_if(patches_.begin(), patches_.end(),
		                              [id](const auto &p) {
			                              return p.src_id == id ||
			                                     p.dst_id == id;
		                              }),
		               patches_.end());
		return true;
	}

	Error connect(AudioNodeID src_id, AudioPort src_port, AudioNodeID dst_id,
	              AudioPort dst_port) {
		if (patches_.is_full()) {
			return Error::PATCHES_EXHAUSTED;
		}

		if (find_patch(src_id, src_port, dst_id, dst_port) != patches_.end()) {
			return Error::PATCH_ALREADY_EXISTS;
		}

		const auto src = find_node(src_id);
		const auto dst = find_node(dst_id);
		if (src == nodes_.end() || dst == nodes_.end()) {
			return Error::NODE_NOT_FOUND;
		}

		patches_.emplace_back(*src, src_port, *dst, dst_port);
		return Error::NONE;
	}

	// Connect two nodes on port 0
	Error connect(AudioNodeID src_id, AudioNodeID dst_id) {
		return connect(src_id, AudioPort{0}, dst_id, AudioPort{0});
	}

	bool disconnect(AudioNodeID src_id, AudioPort src_port, AudioNodeID dst_id,
	                AudioPort dst_port) {
		auto it = find_patch(src_id, src_port, dst_id, dst_port);
		if (it == patches_.end()) {
			return false;
		}
		patches_.erase(it);
		return true;
	}

	// Remove the first connection found between src and dst
	bool disconnect(AudioNodeID src_id, AudioNodeID dst_id) {
		auto it = std::find_if(
		    patches_.begin(), patches_.end(), [src_id, dst_id](const auto &p) {
			    return p.src_id == src_id && p.dst_id == dst_id;
		    });
		if (it == patches_.end()) {
			return false;
		}
		patches_.erase(it);
		return true;
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
	[[nodiscard]] auto &nodes() { return nodes_; }
	[[nodiscard]] auto &patches() { return patches_; }

  private:
	auto find_node(AudioNodeID id) {
		return std::find_if(nodes_.begin(), nodes_.end(),
		                    [id](const auto &n) { return n.id == id; });
	}

	auto find_node(AudioNodeID id) const {
		return std::find_if(nodes_.cbegin(), nodes_.cend(),
		                    [id](const auto &n) { return n.id == id; });
	}

	auto find_patch(AudioNodeID src_id, AudioPort src_port, AudioNodeID dst_id,
	                AudioPort dst_port) const {
		return std::find_if(
		    patches_.cbegin(), patches_.cend(), [=](const auto &p) {
			    return src_id == p.src_id && src_port == p.src_port &&
			           dst_id == p.dst_id && dst_port == p.dst_port;
		    });
	}

	auto find_patch(AudioNodeID src_id, AudioNodeID dst_id) const {
		return std::find_if(patches_.cbegin(), patches_.end(),
		                    [=](const auto &p) {
			                    return src_id == p.src_id && dst_id == p.dst_id;
		                    });
	}

	auto find_patch(AudioNodeID src_id, AudioPort src_port, AudioNodeID dst_id,
	                AudioPort dst_port) {
		return std::find_if(
		    patches_.begin(), patches_.end(), [=](const auto &p) {
			    return src_id == p.src_id && src_port == p.src_port &&
			           dst_id == p.dst_id && dst_port == p.dst_port;
		    });
	}

	AudioNodeID emplace_and_get_id(::AudioStream *device) {
		const auto new_id = node_id_gen_.next_id();
		nodes_.emplace_back(new_id, device);
		return new_id;
	}

	struct {
		AudioNodeID next_id() {
			++last_id.value;
			return AudioNodeID{last_id};
		}
		AudioNodeID last_id{};
	} node_id_gen_;

	nst::inplace_vector<AudioNode, NodeCapacity> nodes_;
	nst::inplace_vector<AudioPatch, PatchCapacity> patches_;
};

} // namespace nst::teensy

#endif