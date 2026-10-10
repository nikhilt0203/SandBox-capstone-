#ifndef SANDBOX_MODULE_TYPES_HPP_
#define SANDBOX_MODULE_TYPES_HPP_

#include "envelope.hpp"
#include "oscillator.hpp"
#include <nst/object_pool.hpp>
#include <nst/strong_alias.hpp>
#include <nst/type_list.hpp>

#include "audio/audio_engine.hpp"

namespace sndbx {

// Modules recognized by the system
using ModuleTypes = nst::type_list<Oscillator, Envelope>;

// Module types that can be created by the user
using ModuleBankTypes = nst::type_list<Oscillator>;

namespace limits {
// Max instances per module, default 16
template <typename Module> inline constexpr std::size_t max_instances = 16;

template <> inline constexpr std::size_t max_instances<Oscillator> = 32;
template <> inline constexpr std::size_t max_instances<Envelope> = 32;
} // namespace limits

// An integer ID for each module type in the ModuleTypes type list
struct ModuleType : public nst::strong_alias<std::size_t, ModuleType> {
	using strong_alias::strong_alias;

	constexpr bool operator==(const ModuleType &other) const noexcept {
		return value == other.value;
	}
	constexpr bool operator!=(const ModuleType &other) const noexcept {
		return value != other.value;
	}
};

// Get the value representation of a module type
template <typename T, typename = std::enable_if_t<ModuleTypes::contains_v<T>>>
inline constexpr ModuleType module_type{ModuleTypes::index_of<T>};

} // namespace sndbx

namespace sndbx {

namespace impl {

template <class Module> auto &module_pool() {
	static nst::object_pool<Module, limits::max_instances<Module>> pool;
	return pool;
}

template <class Module> static void release(void *module) {
	module_pool<Module>().release(static_cast<Module *>(module));
}

using ReleaseTable = std::array<void (*)(void *), ModuleTypes::size>;

template <std::size_t... Is>
inline constexpr auto make_table(std::index_sequence<Is...>) {
	return ReleaseTable{&release<ModuleTypes::get<Is>>...};
}

inline constexpr auto release_table = make_table(ModuleTypes::index_sequence{});

} // namespace impl

namespace arena {

template <class Module, typename... Args>
inline auto acquire_module(Args &&...args) {
	return impl::module_pool<Module>().acquire(std::forward<Args>(args)...);
}

inline void release_module(ModuleType type, void *module) {
	impl::release_table[type.value](module);
}

} // namespace arena

} // namespace sndbx

#endif