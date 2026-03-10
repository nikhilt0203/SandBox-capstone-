#ifndef SANDBOX_ERROR_RESULT_HPP_
#define SANDBOX_ERROR_RESULT_HPP_

#include <cassert>

namespace sndbx
{
  enum class Error
  {
    NONE,
    OUT_OF_RANGE,
    ELEMENT_NOT_FOUND,
    BUILDER_REGISTRY_FULL,
    BUILDER_POOL_EXHAUSTED,
    BUILDER_INVALID_POS
  };

  template<typename T>
  struct [[nodiscard]] Result
  {
    Error error;
    T value{};

    constexpr Result(T value) : error(Error::NONE), value(value) {}

    constexpr Result(Error error) : error(error) {}

    constexpr operator bool() const { return error == Error::NONE; }
  };
}

#endif