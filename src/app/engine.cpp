#include "engine.hpp"

#include <algorithm>
#include <tuple>

#include "modules/patchable.hpp"

namespace sndbx {

// helpers
namespace {

template <typename Container>
ModuleConnection *
find_connection(Container &c, ModulePosition src_pos, std::uint8_t output_idx,
                ModulePosition dst_pos, std::uint8_t input_idx) {
	return std::find_if(
	    c.begin(), c.end(),
	    [src_pos, output_idx, dst_pos, input_idx](const auto &c) {
		    return src_pos == c.src_pos && output_idx == c.output_idx &&
		           dst_pos == c.dst_pos && input_idx == c.input_idx;
	    });
}

template <typename Container>
const ModuleConnection *
find_connection(const Container &c, ModulePosition src_pos,
                std::uint8_t output_idx, ModulePosition dst_pos,
                std::uint8_t input_idx) {
	return std::find_if(
	    c.cbegin(), c.cend(),
	    [src_pos, output_idx, dst_pos, input_idx](const auto &c) {
		    return src_pos == c.src_pos && output_idx == c.output_idx &&
		           dst_pos == c.dst_pos && input_idx == c.input_idx;
	    });
}

} // namespace

namespace audio {
bool connect(const Patchable &src, std::uint8_t output_idx,
             const Patchable &dst, std::uint8_t input_idx, AudioGraph &graph) {
	assert(output_idx < src.num_outs() && input_idx < dst.num_ins());

	const auto [out_id, out_port] =
	    src.patch_point(ModulePort{PortType::OUT, output_idx});
	const auto [in_id, in_port] =
	    dst.patch_point(ModulePort{PortType::IN, input_idx});

	assert(out_id && in_id);
	return graph.connect(out_id, out_port, in_id, in_port);
}

bool disconnect(const Patchable &src, std::uint8_t output_idx,
                const Patchable &dst, std::uint8_t input_idx,
                AudioGraph &graph) {
	assert(output_idx < src.num_outs() && input_idx < dst.num_ins());

	const auto [out_id, out_port] =
	    src.patch_point({PortType::OUT, output_idx});
	const auto [in_id, in_port] = dst.patch_point({PortType::IN, input_idx});

	assert(out_id && in_id);
	return graph.disconnect(out_id, out_port, in_id, in_port);
}
} // namespace audio

bool Engine::connect(ModulePosition src_pos, std::uint8_t output_idx,
                     ModulePosition dst_pos, std::uint8_t input_idx) {
	if (src_pos == dst_pos) {
		return false;
	}

	if (connections_.is_full()) {
		return false;
	}

	if (connection_exists(src_pos, output_idx, dst_pos, input_idx)) {
		return false;
	}

	const auto &src = module_map_[src_pos];
	const auto &dst = module_map_[dst_pos];
	if (!src.holds<Patchable>() || !dst.holds<Patchable>()) {
		return false;
	}

	return audio::connect(src.get<Patchable>(), output_idx,
	                      dst.get<Patchable>(), input_idx, audio_graph_) &&
	       connections_.emplace_back(
	           ModuleConnection{src_pos, output_idx, dst_pos, input_idx});
}

bool Engine::disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                        ModulePosition dst_pos, std::uint8_t input_idx) {
	auto it =
	    find_connection(connections_, src_pos, output_idx, dst_pos, input_idx);
	if (it == connections_.end()) {
		return false;
	}

	const auto &src = module_map_[src_pos];
	const auto &dst = module_map_[dst_pos];
	if (!src.holds<Patchable>() || !dst.holds<Patchable>()) {
		return false;
	}

	if (!audio::disconnect(src.get<Patchable>(), output_idx,
	                       dst.get<Patchable>(), input_idx, audio_graph_)) {
		return false;
	}
	connections_.erase(it);
	return true;
}

namespace {
static std::array<bool, limits::max_in_ports> avail_ins;
static std::array<bool, limits::max_out_ports> avail_outs;
} // namespace

bool Engine::connect_first(ModulePosition src_pos, ModulePosition dst_pos) {
	assert(module_map_.is_occupied(src_pos) &&
	       module_map_.is_occupied(dst_pos));
	avail_ins.fill(true);
	avail_outs.fill(true);

	for (const auto &c : connections_) {
		if (c.src_pos == src_pos) {
			avail_outs[c.output_idx] = false;
		} else if (c.dst_pos == dst_pos) {
			avail_ins[c.input_idx] = false;
		}
	}

	auto first_true_idx = [](const auto &c) -> std::uint8_t {
		return std::distance(
		    c.begin(),
		    std::find_if(c.begin(), c.end(), [](const bool b) { return b; }));
	};

	const auto first_output = first_true_idx(avail_outs);
	if (first_output == avail_outs.size()) {
		return false;
	}
	const auto first_input = first_true_idx(avail_ins);
	if (first_input == avail_ins.size()) {
		return false;
	}

	return connect(src_pos, first_output, dst_pos, first_input);
}

bool Engine::disconnect_first(ModulePosition src_pos, ModulePosition dst_pos) {
	return false;
}

bool Engine::connection_exists(ModulePosition src_pos,
                               ModulePosition dst_pos) const {
	for (const auto &c : connections_) {
		if (src_pos == c.src_pos && dst_pos == c.dst_pos) {
			return true;
		}
	}
	return false;
}

bool Engine::connection_exists(ModulePosition src_pos, std::uint8_t output_idx,
                               ModulePosition dst_pos,
                               std::uint8_t input_idx) const {
	for (const auto &c : connections_) {
		if (src_pos == c.src_pos && dst_pos == c.dst_pos &&
		    c.input_idx == input_idx && c.output_idx == output_idx) {
			return true;
		}
	}
	return false;
}

} // namespace sndbx