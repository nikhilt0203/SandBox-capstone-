#ifndef SANDBOX_MODULE_TYPES_HPP_
#define SANDBOX_MODULE_TYPES_HPP_

#include "oscillator.hpp"
#include <nst/object_pool.hpp>
#include <nst/strong_alias.hpp>
#include <nst/type_list.hpp>

#include "audio/audio_engine.hpp"

namespace sndbx {

// Modules recognized by the system
using ModuleTypes = nst::type_list<Oscillator>;

// Max instances per module
namespace limits {
template <typename Module> constexpr std::size_t max_instances = 16U;
template <> constexpr auto max_instances<Oscillator> = 32;
} // namespace limits

// compile-time type id
struct ModuleType : public nst::strong_alias<std::size_t, ModuleType> {
  using strong_alias::strong_alias;

  constexpr bool operator==(const ModuleType &other) const {
    return value == other.value;
  }
  constexpr bool operator!=(const ModuleType &other) const {
    return value != other.value;
  }
};

// type lookup
template <typename T, typename = std::enable_if_t<ModuleTypes::contains<T>>>
constexpr ModuleType module_type{ModuleTypes::index_of<T>};

} // namespace sndbx

namespace sndbx::engine {

namespace impl {

template <typename T> auto &module_pool() {
  static nst::object_pool<T, limits::max_instances<T>> pool;
  return pool;
}

template <typename T> static void release(void *m) {
  module_pool<T>().release(static_cast<T *>(m));
}

using ReleaseTable = std::array<void (*)(void *), ModuleTypes::size>;

template <std::size_t... Is>
static constexpr ReleaseTable make_table(std::index_sequence<Is...>) {
  return {&release<ModuleTypes::get<Is>>...};
}

inline static constexpr ReleaseTable release_table =
    make_table(ModuleTypes::index_sequence{});
} // namespace impl

template <class Module, typename... Args>
inline auto acquire_module(Args &&...args) {
  return impl::module_pool<Module>().acquire(std::forward<Args>(args)...);
}

inline void release_module(ModuleType type, void *module) {
  impl::release_table[type.value](module);
}

} // namespace sndbx::engine

#endif