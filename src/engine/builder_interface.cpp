#include "builder_interface.hpp"

#define MODULE_TYPE(n) case n: return builder.make<ModuleBank::get<n>>(pos).error

sndbx::Error sndbx::engine::createModuleFromBankIndex(std::size_t bankIndex, grid::Position pos, ModuleBuilder& builder) 
{
  switch (bankIndex) 
  {
    MODULE_TYPE(0);
    MODULE_TYPE(1);
    MODULE_TYPE(2);
    MODULE_TYPE(3);
    MODULE_TYPE(4);
    MODULE_TYPE(5);
    MODULE_TYPE(6);
    MODULE_TYPE(7);
    default: return Error::NONE;
  }
}
