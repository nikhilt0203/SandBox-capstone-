#ifndef SANDBOX_MODULE_INTERFACES_HPP_
#define SANDBOX_MODULE_INTERFACES_HPP_

#include <string_view>
#include "Adafruit_GFX.h"
#include <cstdint>
#include "core/fixed_string.hpp"
#include "core/fixed_vector.hpp"

#define MODULE_TYPE_INFO(name, description, color) \
  static constexpr std::string_view NAME = name; \
  static constexpr std::string_view DESCRIPTION = description; \
  static constexpr std::uint32_t COLOR = color

class Serializable
{
public:
  virtual ~Serializable() = default;
  [[nodiscard]] virtual std::string toString() const = 0;
};

class Controllable
{
public:
  virtual ~Controllable() = default;

  [[nodiscard]] virtual std::size_t numControls() const = 0;
  virtual void changeControl(std::size_t index, int delta) = 0;
  virtual void resetControls() {}
};

template<typename T, std::size_t N>
struct EmptyVector { inline static const sndbx::fixed_vector<T, N> value{}; };

class Displayable
{
public:
  virtual ~Displayable() = default;

  [[nodiscard]] virtual std::uint32_t displayColor() const { return 0xFFFFFF; }
  [[nodiscard]] virtual std::uint32_t ledColor() const { return displayColor(); };

  [[nodiscard]] virtual std::string_view displayName() const { return "unnamed"; }

  [[nodiscard]] virtual auto inputNames() const 
    -> const sndbx::vector_8U<std::string_view>& { return EmptyVector<std::string_view, 8>::value; }

  [[nodiscard]] virtual auto outputNames() const 
    -> const sndbx::vector_8U<std::string_view>& { return EmptyVector<std::string_view, 8>::value; }

  [[nodiscard]] virtual auto controlNames() const 
    -> const sndbx::vector_4U<std::string_view>& { return EmptyVector<std::string_view, 4>::value; }

  [[nodiscard]] virtual auto normalizedControlValues() const 
    -> const sndbx::vector_4U<float>& { return EmptyVector<float, 4>::value; }
};

class Pressable
{
public:
  virtual ~Pressable() = default;

  virtual void onRisingEdge() = 0;
  virtual void onFallingEdge() = 0;
};

class Animatable
{
public:
  virtual ~Animatable() = default;
  virtual void drawNext(GFXcanvas16& frame) const = 0;
};

#endif