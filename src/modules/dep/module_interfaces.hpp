#ifndef SANDBOX_MODULE_INTERFACES_HPP_
#define SANDBOX_MODULE_INTERFACES_HPP_

#include <string_view>
#include "Adafruit_GFX.h"
#include <stdint.h>

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

  virtual void changeControl(std::size_t index, int delta) = 0;
  [[nodiscard]] virtual std::size_t numControls() const = 0;
};

class Displayable
{
public:
  virtual ~Displayable() = default;

  [[nodiscard]] virtual std::uint32_t displayColor() const = 0;
  [[nodiscard]] virtual std::uint32_t ledColor() const { return displayColor(); };
  [[nodiscard]] virtual std::string_view displayName() const = 0;
  [[nodiscard]] virtual auto controlNames() const -> const std::vector<std::string_view>& = 0;
  [[nodiscard]] virtual auto inputNames() const -> const std::vector<std::string_view>& = 0;
  [[nodiscard]] virtual auto outputNames() const -> const std::vector<std::string_view>& = 0;
  [[nodiscard]] virtual auto normalizedControlValues() const -> const std::vector<float>& = 0;
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

[[nodiscard]] inline const std::vector<std::string_view>& emptyVectorSV()
{ 
  static const std::vector<std::string_view> empty{};
  return empty; 
}

[[nodiscard]] inline const std::vector<float>& emptyVectorFloat()
{ 
  static const std::vector<float> empty{};
  return empty; 
}

#endif