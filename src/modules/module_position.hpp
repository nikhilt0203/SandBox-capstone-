#ifndef SANDBOX_MODULE_POSITION_HPP_
#define SANDBOX_MODULE_POSITION_HPP_

#include <cstddef>
#include <nst/strong_alias.hpp>

namespace sndbx {

struct ModulePosition : public nst::strong_alias<std::size_t, ModulePosition> {
  using strong_alias::strong_alias;

  constexpr bool operator==(const ModulePosition &other) const {
    return value == other.value;
  }
  constexpr bool operator!=(const ModulePosition &other) const {
    return value != other.value;
  }
};

} // namespace sndbx

#endif