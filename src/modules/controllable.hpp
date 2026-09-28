#ifndef SANDBOX_CONTROLLABLE_HPP_
#define SANDBOX_CONTROLLABLE_HPP_

#include <cstdint>

namespace sndbx {
class Controllable {
public:
  virtual ~Controllable() = default;
  virtual void change(std::uint8_t ctrl, std::int8_t amt) = 0;
};
} // namespace sndbx

#endif