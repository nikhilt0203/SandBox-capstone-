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

sndbx::Error createModuleFromBankIndex(std::size_t bankIndex, grid::Position pos, ModuleBuilder& builder);

}

#endif