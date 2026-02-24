#ifndef module_interfaces_hpp_
#define module_interfaces_hpp_

#include <string_view>
#include "Adafruit_GFX.h"
#include <stdint.h>

#define MODULE_TYPE_INFO(name, description, color) \
  static constexpr std::string_view NAME = name; \
  static constexpr std::string_view DESCRIPTION = description; \
  static constexpr std::uint32_t COLOR = color 

inline static const std::vector<std::string_view> EMPTY{};
inline static const std::vector<float> EMPTYF{};

class Controllable
{
public:
  virtual ~Controllable() = default;

  virtual void changeControl(std::size_t index, int delta) = 0;
  [[nodiscard]] virtual std::size_t numControls() = 0;
};

class Displayable
{
public:
  virtual ~Displayable() = default;

  [[nodiscard]] virtual std::uint32_t displayColor() const { return 0xFFFFFF; };
  [[nodiscard]] virtual std::uint32_t ledColor() const { return displayColor(); };
  [[nodiscard]] virtual std::string_view displayName() const { return "unnamed"; }
  [[nodiscard]] virtual const std::vector<std::string_view>& controlNames() const = 0;
  [[nodiscard]] virtual const std::vector<std::string_view>& inputNames() const = 0;
  [[nodiscard]] virtual const std::vector<std::string_view>& outputNames() const = 0;
  [[nodiscard]] virtual const std::vector<float>& normalizedControlValues() const = 0;
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