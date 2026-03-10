#ifndef SANDBOX_MODULE_TYPES_HPP_
#define SANDBOX_MODULE_TYPES_HPP_

#include "modules/envelope.hpp"
#include "modules/keyboard.hpp"
#include "modules/lfo.hpp"
#include "modules/mixer.hpp"
#include "modules/mult.hpp"
#include "modules/oscillator.hpp"
#include "modules/oscilloscope.hpp"
#include "modules/reverb.hpp"
#include "modules/usbout.hpp"
#include "modules/vcf.hpp"

#include "core/object_pool.hpp"
#include "core/type_array.hpp"

namespace sndbx::engine
{

using ModuleBank = 
  sndbx::type_array<
    Oscillator, 
    LFO,
    Mixer, 
    USBOut, 
    Envelope, 
    VCF, 
    Keyboard, 
    Mult,
    Oscilloscope>;

using ModuleTypes = 
  sndbx::type_array<
    Oscillator, 
    LFO,
    Mixer, 
    USBOut, 
    Envelope, 
    VCF, 
    Keyboard, 
    Oscilloscope,
    Mult,
    KeyboardKey>;

template<typename T> struct PoolSize { static constexpr auto max = 16U; };
#define MAX_COUNT(type, count) template<> struct PoolSize<type> { static constexpr std::size_t max = count; }

MAX_COUNT(Oscillator, 32);
MAX_COUNT(LFO, 32);
MAX_COUNT(Mixer, 16);
MAX_COUNT(USBOut, 1);
MAX_COUNT(Envelope, 16);
MAX_COUNT(VCF, 16);
MAX_COUNT(Keyboard, 5);
MAX_COUNT(Mult, 16);
MAX_COUNT(Oscilloscope, 8);
MAX_COUNT(KeyboardKey, 54); //max keys 32, highest possible total possible is 2 keyboards (len 32 + len 22)

struct ModulePools
{
  template<typename T>
  auto& pool()
  {
    static sndbx::object_pool<T, PoolSize<T>::max> pool;
    return pool;
  }

  template<typename T>
  T* acquire() { return pool<T>().acquire(); }

  template<typename T>
  void release(T* obj) { pool<T>().release(obj); }
};

template <typename T> 
constexpr std::size_t typeIndexOf() { return indexOf<T, ModuleTypes>(); }
template <typename T> 
constexpr std::size_t bankIndexOf() { return indexOf<T, ModuleBank>(); }

constexpr std::size_t numModuleTypes() { return ModuleTypes::size; }

}

#endif