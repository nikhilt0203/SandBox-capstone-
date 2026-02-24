#ifndef builder_interface_hpp_
#define builder_interface_hpp_

#include "engine/module_builder.hpp"
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

#include <cstddef>
#include <tuple>
#include <utility>
#pragma once

using ModuleTypes = 
  sndbx::TypeArray<
    Oscillator, 
    LFO,
    Mixer, 
    USBOut, 
    Envelope, 
    VCF, 
    Keyboard, 
    Oscilloscope>;

struct ModuleBankEntry 
{
  std::string_view name;
  std::string_view description;
  std::uint32_t color;
};

template <std::size_t... Is>
constexpr std::array<ModuleBankEntry, ModuleTypes::size> createBankInfoArray(std::index_sequence<Is...>) 
{
  return {ModuleBankEntry{ModuleTypes::get<Is>::NAME,
                          ModuleTypes::get<Is>::DESCRIPTION,
                          ModuleTypes::get<Is>::COLOR}...};
};

inline static constexpr auto bankInfos =
    createBankInfoArray(std::make_index_sequence<ModuleTypes::size>{});

namespace sndbx::engine 
{

template <typename T> constexpr std::size_t typeIndexOf() { return indexOf<T, ModuleTypes>(); }

void buildFromTypeIndex(std::size_t index, grid::Position pos, ModuleBuilder &builder) 
{
  if (index >= ModuleTypes::size) { return; }

  switch (index) 
  {
  case 0:
    builder.make<ModuleTypes::get<0>>(pos);
    break;
  case 1:
    builder.make<ModuleTypes::get<1>>(pos);
    break;
  case 2:
    builder.make<ModuleTypes::get<2>>(pos);
    break;
  case 3:
    builder.make<ModuleTypes::get<3>>(pos);
    break;
  case 4:
    builder.make<ModuleTypes::get<4>>(pos);
    break;
  case 5:
    builder.make<ModuleTypes::get<5>>(pos);
    break;
  case 6:
    builder.make<ModuleTypes::get<6>>(pos);
    break;
  case 7:
    builder.make<ModuleTypes::get<7>>(pos);
    break;
  default:
    break;
  }
}
} // namespace sndbx::engine

#endif