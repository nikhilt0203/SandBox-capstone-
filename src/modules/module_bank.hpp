#ifndef SANDBOX_MODULE_BANK_HPP_
#define SANDBOX_MODULE_BANK_HPP_

#include "module_display_info.hpp"
#include "module_types.hpp"

namespace sndbx {

/*
 * Modules listed in the bank
 */
using ModuleBankTypes = nst::type_list<Oscillator>;

template <typename T,
          typename = std::enable_if_t<ModuleBankTypes::contains_v<T>>>
inline constexpr std::size_t bank_index = ModuleBankTypes::index_of<T>;

struct ModuleBankEntry {
	ModuleType type;
	ModuleName name;
	ModuleDescription description;
};

using ModuleBank = std::array<ModuleBankEntry, ModuleBankTypes::size>;

template <std::size_t... Is>
static constexpr ModuleBank make_module_bank(std::index_sequence<Is...>) {
	return ModuleBank{[]() {
		using Module = ModuleTypes::get<Is>;
		return ModuleBankEntry{module_type<Module>, module_info<Module>.name,
		                       module_info<Module>.description};
	}()...};
}

} // namespace sndbx

#endif