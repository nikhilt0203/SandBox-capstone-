#include "engine.hpp"
#include <algorithm>
#include <tuple>

namespace sndbx {

// helpers
namespace {

template <std::size_t... Is>
std::optional<ModuleID> create_module_impl(ModuleFactory factory,
                                           ModuleType type, ModulePosition pos,
                                           std::index_sequence<Is...>) {
	auto to_optional_id = [](const auto &exp) -> std::optional<ModuleID> {
		if (exp) {
			return exp->id;
		}
		return std::nullopt;
	};

	std::optional<ModuleID> ret{};
	// short circuits if 'type' is valid + .make() succeeds
	((ModuleType{Is} == type &&
	  (ret = to_optional_id(factory.make<ModuleTypes::get<Is>>(pos)))) ||
	 ...);
	return ret;
}

template <typename Container>
auto find_connection(Container &c, ModulePosition src_pos,
                     std::uint8_t output_idx, ModulePosition dst_pos,
                     std::uint8_t input_idx) -> ModuleConnection * {
	return std::find_if(
	    c.begin(), c.end(),
	    [src_pos, output_idx, dst_pos, input_idx](const auto &c) {
		    return src_pos == c.src_pos && output_idx == c.output_idx &&
		           dst_pos == c.dst_pos && input_idx == c.input_idx;
	    });
}

template <typename Container>
auto find_connection(const Container &c, ModulePosition src_pos,
                     std::uint8_t output_idx, ModulePosition dst_pos,
                     std::uint8_t input_idx) -> const ModuleConnection * {
	return std::find_if(
	    c.cbegin(), c.cend(),
	    [src_pos, output_idx, dst_pos, input_idx](const auto &c) {
		    return src_pos == c.src_pos && output_idx == c.output_idx &&
		           dst_pos == c.dst_pos && input_idx == c.input_idx;
	    });
}

auto get_patchable_pair(ModuleFactory factory, ModuleID src, ModuleID dst)
    -> std::optional<
        std::pair<const audio::Patchable *, const audio::Patchable *>> {
	auto m1 = factory.get_module(src);
	if (!m1 || !m1->holds<audio::Patchable>()) {
		return std::nullopt;
	}
	auto m2 = factory.get_module(dst);
	if (!m2 || !m2->holds<audio::Patchable>()) {
		return std::nullopt;
	}
	return std::make_pair(static_cast<const audio::Patchable *>(*m1),
	                      static_cast<const audio::Patchable *>(*m2));
}

} // namespace

bool Engine::connect(ModulePosition src_pos, std::uint8_t output_idx,
                     ModulePosition dst_pos, std::uint8_t input_idx) {
	if (src_pos == dst_pos) {
		return false;
	}
	if (connections_.is_full()) {
		return false;
	}
	if (find_connection(connections_, src_pos, output_idx, dst_pos,
	                    input_idx) != connections_.end()) {
		return false;
	}

	const auto src_id = factory_[src_pos];
	const auto dst_id = factory_[dst_pos];

	auto modules = get_patchable_pair(factory_, src_id, dst_id);
	if (!modules) {
		return false;
	}

	if (AudioError::NONE != audio::connect(*modules->first, output_idx,
	                                       *modules->second, input_idx,
	                                       audio_graph_)) {
		return false;
	}

	connections_.emplace_back(
	    ModuleConnection{src_pos, output_idx, dst_pos, input_idx});
	return true;
}

bool Engine::disconnect(ModulePosition src_pos, std::uint8_t output_idx,
                        ModulePosition dst_pos, std::uint8_t input_idx) {
	if (src_pos == dst_pos) {
		return false;
	}

	const auto src_id = factory_[src_pos];
	const auto dst_id = factory_[dst_pos];

	auto modules = get_patchable_pair(factory_, src_id, dst_id);
	if (!modules) {
		return false;
	}

	if (AudioError::NONE != audio::disconnect(*modules->first, output_idx,
	                                          *modules->second, input_idx,
	                                          audio_graph_)) {
		return false;
	}

	auto it =
	    find_connection(connections_, src_pos, output_idx, dst_pos, input_idx);
	assert(it != connections_.end());
	connections_.erase(it);
	return true;
}

std::optional<ModuleID> Engine::create_module(ModuleType type,
                                              ModulePosition pos) {
	if (!create_module_impl(factory_, type, pos,
	                        ModuleTypes::index_sequence{})) {
		return std::nullopt;
	}

	const auto id = factory_[pos];
	auto &module = factory_[id];

	// initialize audio if module has an audio component
	if (module.holds<audio::Patchable>() &&
	    (AudioError::NONE !=
	     module.get<audio::Patchable>().link(audio_graph_))) {
		factory_.erase(id);
		return std::nullopt;
	}

	return id;
}

namespace {
static std::array<bool, limits::max_in_ports> avail_ins;
static std::array<bool, limits::max_out_ports> avail_outs;
} // namespace

bool Engine::connect_first(ModulePosition src_pos, ModulePosition dst_pos) {
	const auto src_id = factory_[src_pos];
	const auto dst_id = factory_[dst_pos];

	auto modules = get_patchable_pair(factory_, src_id, dst_id);
	if (!modules) {
		return false;
	}

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

bool Engine::delete_module(ModulePosition pos) {
	auto id = factory_.get_id(pos);
	if (!id) {
		return false;
	}

	// remove from audio system if module has an audio component
	auto &module = factory_[*id];
	if (module.holds<audio::Patchable>()) {
		module.get<audio::Patchable>().unlink(audio_graph_);
	}

	factory_.erase(*id);
	return true;
}

bool Engine::connection_exists(ModulePosition src_pos,
                               ModulePosition dst_pos) const {
	return std::find_if(connections_.begin(), connections_.end(),
	                    [src_pos, dst_pos](const auto &c) {
		                    return src_pos == c.src_pos && dst_pos == c.dst_pos;
	                    }) != connections_.end();
}

bool Engine::connection_exists(ModulePosition src_pos, std::uint8_t output_idx,
                               ModulePosition dst_pos,
                               std::uint8_t input_idx) const {
	return find_connection(connections_, src_pos, output_idx, dst_pos,
	                       input_idx) != connections_.end();
}

} // namespace sndbx