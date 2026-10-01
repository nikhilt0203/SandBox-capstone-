#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include <array>

namespace sndbx {

template <std::size_t MaxBytes> class Serializable {
  using Buffer = std::array<std::byte, MaxBytes>;

  virtual ~Serializable() = default;
  virtual void serialize(Buffer &) const = 0;
};

} // namespace sndbx

#endif