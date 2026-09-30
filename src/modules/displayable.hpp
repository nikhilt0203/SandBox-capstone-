#ifndef SANDBOX_DISPLAYABLE_HPP_
#define SANDBOX_DISPLAYABLE_HPP_

namespace sndbx {
class Displayable {
public:
  virtual ~Displayable() = default;
  virtual std::string_view display_name() const = 0;
  virtual std::uint16_t display_color() const = 0;
};
} // namespace sndbx

#endif