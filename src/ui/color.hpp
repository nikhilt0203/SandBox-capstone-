#ifndef color_hpp_
#define color_hpp_

#include <cstdint>

namespace sndbx::color
{
  struct RGBColor
  {
    std::uint8_t r, g, b;
    [[nodiscard]] constexpr std::uint32_t hex() const noexcept { return (r << 16) | (g << 8) | b; }
  };

  [[nodiscard]] constexpr RGBColor toRGB(std::uint32_t hex) noexcept
  { 
    return RGBColor{
      static_cast<std::uint8_t>((hex >> 16) & 0xFF), 
      static_cast<std::uint8_t>((hex >> 8) & 0xFF), 
      static_cast<std::uint8_t>(hex & 0xFF)
    }; 
  }

  [[nodiscard]] constexpr std::uint16_t to565(std::uint32_t hex888) noexcept
  {
    std::uint8_t r = (hex888 >> 16) & 0xFF;
    std::uint8_t g = (hex888 >> 8) & 0xFF;
    std::uint8_t b = (hex888 & 0xFF);
    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
  }

  [[nodiscard]] constexpr std::uint32_t changeBrightness(std::uint32_t color, float brightness) noexcept
  {
    auto r = static_cast<std::uint8_t>((color >> 16) & 0xFF);
    auto g = static_cast<std::uint8_t>((color >> 8) & 0xFF);
    auto b = static_cast<std::uint8_t>(color & 0xFF);

    r *= brightness;
    g *= brightness;
    b *= brightness;

    return static_cast<std::uint32_t>((r << 16) | (g << 8) | b);
  }
}

#endif