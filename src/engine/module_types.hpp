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
#include "object_pool.hpp"
#include "type_array.hpp"

namespace sndbx::engine
{

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
MAX_COUNT(KeyboardKey, 55);

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

template <typename T> 
constexpr std::size_t bankIndexOf() { return indexOf<T, ModuleBank>(); }

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

template <typename T> 
constexpr std::size_t typeIndexOf() { return indexOf<T, ModuleTypes>(); }

struct ModulePools
{
  template<typename T>
  auto& pool()
  {
    static object_pool<T, PoolSize<T>::max> pool;
    return pool;
  }

  template<typename T>
  T* acquire() { return pool<T>().acquire(); }

  template<typename T>
  void release(T* obj) { pool<T>().release(obj); }
};

}

#endif