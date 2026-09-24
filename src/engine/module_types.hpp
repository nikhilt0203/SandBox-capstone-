#ifndef SANDBOX_MODULE_TYPES_HPP_
#define SANDBOX_MODULE_TYPES_HPP_

#include <nst/object_pool.hpp>
#include <nst/strong_alias.hpp>
#include <nst/type_list.hpp>

#include "modules/combine.hpp"
#include "modules/envelope.hpp"
#include "modules/keyboard.hpp"
#include "modules/lfo.hpp"
#include "modules/mixer.hpp"
#include "modules/mult.hpp"
#include "modules/oscillator.hpp"
#include "modules/oscilloscope.hpp"
#include "modules/reverb.hpp"
#include "modules/sequencer.hpp"
#include "modules/usbout.hpp"
#include "modules/vcf.hpp"

namespace sndbx {

using ModuleTypes =
    nst::type_list<Oscillator, LFO, Mixer, USBOut, Envelope, VCF, Keyboard,
                   Oscilloscope, Mult, KeyboardKey, Sequencer, SequencerStep,
                   Combine>;

using ModuleBank =
    nst::type_list<Oscillator, LFO, Mixer, USBOut, Envelope, Combine, VCF,
                   Keyboard, Mult, Oscilloscope, Sequencer>;

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
constexpr ModuleType type_id{ModuleTypes::index_of<T>};

template <std::size_t I, typename = std::enable_if_t<(I < ModuleTypes::size)>>
using get_type = ModuleTypes::get<I>;

template <typename T, typename = std::enable_if_t<ModuleBank::contains<T>>>
constexpr std::size_t bank_index = ModuleBank::index_of<T>;

} // namespace sndbx

namespace sndbx::engine {

// T must have a static max_count member
class ModulePools {
public:
  template <typename T> T *acquire() { return pool<T>().acquire(); }

  template <typename T> void release(T *obj) {
    if constexpr (std::is_base_of_v<Controllable, T>) {
      obj->resetControls();
    }
    pool<T>().release(obj);
  }

private:
  template <typename T> auto &pool() {
    static nst::object_pool<T, T::max_count> pool;
    return pool;
  }
};

namespace impl {

inline static ModulePools module_pools;

template <typename T> static void release(void *m) {
  module_pools.release(static_cast<T *>(m));
}

using ReleaseTable = std::array<void (*)(void *), ModuleTypes::size>;

template <std::size_t... Is>
static constexpr ReleaseTable make_table(std::index_sequence<Is...>) {
  return {&release<ModuleTypes::get<Is>>...};
}

inline static constexpr ReleaseTable release_table =
    make_table(ModuleTypes::index_sequence{});
} // namespace impl

template <class Module> inline auto pool_acquire() {
  return impl::module_pools.acquire<Module>();
}

inline void pool_release(ModuleType type, void *module) {
  impl::release_table[type.value](module);
}

} // namespace sndbx::engine

#endif