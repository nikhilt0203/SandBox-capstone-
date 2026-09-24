#include "builder_interface.hpp"

#define BANK_TYPE(n)                                                           \
  case n:                                                                      \
    return builder.make<ModuleBank::get<n>>(pos).error_or(                     \
        ModuleBuilder::Error::NONE)

template <std::size_t... Is>
auto create_impl(std::size_t bank_index, const sndbx::grid::Position &pos,
                 ModuleBuilder &builder, std::index_sequence<Is...>) {
  constexpr static auto success = ModuleBuilder::Error::NONE;
  ModuleBuilder::Error out_error;

  (((Is == bank_index) &&
    ((out_error = builder.make<sndbx::get_type<Is>>(pos).error_or(success)) ==
     success)) ||
   ...);

  return out_error;
}

ModuleBuilder::Error sndbx::engine::createModuleFromBankIndex(
    std::size_t bankIndex, const sndbx::grid::Position &pos, ModuleBuilder &builder) {
  return create_impl(bankIndex, pos, builder,
                     sndbx::ModuleBank::index_sequence{});
}
