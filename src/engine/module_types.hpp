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
#include "type_array.hpp"

namespace sndbx::engine
{

using ModuleBank = 
  sndbx::TypeArray<
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
  sndbx::TypeArray<
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
}
#endif