#ifndef SANDBOX_MODULE_INTERFACES_HPP_
#define SANDBOX_MODULE_INTERFACES_HPP_

#include <cstdint>
#include <string_view>

#include "Adafruit_GFX.h"
#include "module.hpp"
#include <nst/inplace_vector.hpp>

#define MODULE_TYPE_INFO(name, description, color)                             \
  static constexpr std::string_view NAME = name;                               \
  static constexpr std::string_view DESCRIPTION = description;                 \
  static constexpr std::uint32_t COLOR = color

class Serializable {
public:
  virtual ~Serializable() = default;
  [[nodiscard]] virtual std::string toString() const = 0;
};

class Controllable {
public:
  static constexpr std::size_t maxControls = 4;
  using ControlValues = nst::inplace_vector<float, maxControls>;

  virtual ~Controllable() = default;

  virtual void changeControl(std::size_t index, int delta) = 0;
  virtual void setControls(const ControlValues &values) {}
  virtual void resetControls() = 0;
};

template <typename T, std::size_t N> struct EmptyVector {
  static inline const nst::inplace_vector<T, N> value{};
};

class Displayable {
public:
  using PortNames = nst::inplace_vector<std::string_view, Module::maxPorts>;
  using ControlNames =
      nst::inplace_vector<std::string_view, Controllable::maxControls>;

  virtual ~Displayable() = default;

  [[nodiscard]] virtual std::uint32_t displayColor() const { return 0xFFFFFF; }
  [[nodiscard]] virtual std::uint32_t ledColor() const {
    return displayColor();
  };

  [[nodiscard]] virtual std::string_view displayName() const {
    return "unnamed";
  }

  [[nodiscard]] virtual auto inputNames() const -> const PortNames & {
    return EmptyVector<std::string_view, Module::maxPorts>::value;
  }

  [[nodiscard]] virtual auto outputNames() const -> const PortNames & {
    return EmptyVector<std::string_view, Module::maxPorts>::value;
  }

  [[nodiscard]] virtual auto controlNames() const -> const ControlNames & {
    return EmptyVector<std::string_view, Controllable::maxControls>::value;
  }

  [[nodiscard]] virtual auto normalizedControlValues() const
      -> const Controllable::ControlValues & {
    return EmptyVector<float, Controllable::maxControls>::value;
  }
};

class Pressable {
public:
  virtual ~Pressable() = default;

  virtual void onRisingEdge() = 0;
  virtual void onFallingEdge() = 0;
};

class Animatable {
public:
  virtual ~Animatable() = default;
  virtual void drawNext(GFXcanvas16 &frame) const = 0;
};

#endif