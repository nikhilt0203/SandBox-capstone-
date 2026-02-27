#ifndef SANDBOX_BUILDER_INTERFACE_HPP_
#define SANDBOX_BUILDER_INTERFACE_HPP_

#include "engine/module_builder.hpp"
#include "engine/module_types.hpp"
#include <cstdint>
#include <tuple>
#include <utility>
#pragma once

namespace sndbx::engine 
{

struct ModuleBankEntry 
{
  std::string_view name;
  std::string_view description;
  std::uint32_t color;
};

template <std::size_t... Is>
constexpr std::array<ModuleBankEntry, ModuleBank::size> createBankInfoArray(std::index_sequence<Is...>) 
{
  return {ModuleBankEntry{ModuleBank::get<Is>::NAME,
                          ModuleBank::get<Is>::DESCRIPTION,
                          ModuleBank::get<Is>::COLOR}...};
};

inline constexpr auto bankInfos = createBankInfoArray(std::make_index_sequence<ModuleBank::size>{});

void buildFromBankIndex(std::size_t index, grid::Position pos, ModuleBuilder& builder) 
{
  if (index >= bankInfos.size()) { return; }

  switch (index) 
  {
  case 0:
    builder.make<ModuleBank::get<0>>(pos);
    break;
  case 1:
    builder.make<ModuleBank::get<1>>(pos);
    break;
  case 2:
    builder.make<ModuleBank::get<2>>(pos);
    break;
  case 3:
    builder.make<ModuleBank::get<3>>(pos);
    break;
  case 4:
    builder.make<ModuleBank::get<4>>(pos);
    break;
  case 5:
    builder.make<ModuleBank::get<5>>(pos);
    break;
  case 6:
    builder.make<ModuleBank::get<6>>(pos);
    break;
  case 7:
    builder.make<ModuleBank::get<7>>(pos);
    break;
  default:
    break;
  }
}

}

#endif