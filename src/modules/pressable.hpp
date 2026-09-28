#ifndef SANDBOX_PRESSABLE_HPP_
#define SANDBOX_PRESSABLE_HPP_

namespace sndbx {
  
class Pressable {
public:
  virtual ~Pressable() = default;
  virtual void on_rising_edge() = 0;
  virtual void on_falling_edge() = 0;
};

} // namespace sndbx

#endif