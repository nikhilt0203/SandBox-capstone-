#ifndef SANDBOX_MODULE_BANK_HPP_
#define SANDBOX_MODULE_BANK_HPP_

#include "module_types.hpp"

namespace sndbx {

/*
 * Modules listed in the bank
 */
using ModuleBankTypes = nst::type_list<Oscillator>;

template <typename T, typename = std::enable_if_t<ModuleBankTypes::contains<T>>>
constexpr std::size_t bank_index = ModuleBankTypes::index_of<T>;

struct ModuleBankEntry {
  ModuleType type;
  std::string_view name;
  std::string_view description;
};

using ModuleBank = std::array<ModuleBankEntry, ModuleBankTypes::size>;

template <std::size_t... Is>
static constexpr ModuleBank make_module_bank(std::index_sequence<Is...>) {
  return ModuleBank{[]() {
    using Module = ModuleTypes::get<Is>;
    constexpr auto name = Module::name;
    constexpr auto desc = Module::desc;

    static_assert(name.size() < limits::module_name_len,
                  "Module name too long");
    static_assert(desc.size() < limits::module_description_len,
                  "Module description too long");

    return ModuleBankEntry{module_type<Module>, name, desc};
  }()...};
}

} // namespace sndbx

#endif