#ifndef SANDBOX_MODULE_ID_HPP_
#define SANDBOX_MODULE_ID_HPP_

#include <cstdint>
#include <nst/strong_alias.hpp>

namespace sndbx {

struct ModuleID : public nst::strong_alias<std::uint32_t, ModuleID> {
  using strong_alias::strong_alias;

  constexpr bool operator==(const ModuleID &other) const {
    return value == other.value;
  }
  constexpr bool operator!=(const ModuleID &other) const {
    return value != other.value;
  }
};

} // namespace sndbx

#endif