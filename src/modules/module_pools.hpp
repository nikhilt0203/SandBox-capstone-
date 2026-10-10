#ifndef SANDBOX_MODULE_POOLS_HPP_
#define SANDBOX_MODULE_POOLS_HPP_

#include "module_types.hpp"
#include <cstddef>

namespace sndbx {
namespace detail {

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

} // namespace detail

namespace arena {

template <class Module, typename... Args>
inline auto acquire_module(Args &&...args) {
	return detail::module_pool<Module>().acquire(std::forward<Args>(args)...);
}

inline void release_module(ModuleType type, void *module) {
	detail::release_table[type.value](module);
}

} // namespace arena

} // namespace sndbx

#endif