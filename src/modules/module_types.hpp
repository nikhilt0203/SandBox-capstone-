#ifndef SANDBOX_MODULE_TYPES_HPP_
#define SANDBOX_MODULE_TYPES_HPP_

#include <nst/object_pool.hpp>
#include <nst/strong_alias.hpp>
#include <nst/type_list.hpp>

#include "audio/audio_engine.hpp"
#include "envelope.hpp"
#include "oscillator.hpp"

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

#endif