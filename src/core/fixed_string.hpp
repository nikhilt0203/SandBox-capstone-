#ifndef SANDBOX_FIXED_STRING_HPP_
#define SANDBOX_FIXED_STRING_HPP_

#include <cstdint>
#include <string_view>

namespace sndbx
{

// Fixed-capacity owning string. constexpr-capable
template<std::size_t N = 32>
class fixed_string
{
public:
  constexpr fixed_string() noexcept = default;

  template<std::size_t ArrSize, typename = std::enable_if_t<ArrSize - 1 <= N>>
  constexpr fixed_string(const char (&s)[ArrSize]) noexcept 
  {
    for (std::size_t i{}; i < ArrSize; i++) { m_Buffer[i] = s[i]; }
    m_Size = ArrSize - 1;
  }

  constexpr explicit fixed_string(const char* s) noexcept { append(s); }

  constexpr explicit fixed_string(std::string_view s) noexcept { append(s); }

  [[nodiscard]] constexpr const char* data() const noexcept { return m_Buffer; }

  [[nodiscard]] constexpr std::string_view view() const noexcept { return std::string_view(data(), m_Size); } 

  [[nodiscard]] constexpr char operator[](std::size_t index) const noexcept { return m_Buffer[index]; } 

  [[nodiscard]] constexpr std::size_t size() const noexcept { return m_Size; }
  [[nodiscard]] static constexpr std::size_t capacity() noexcept { return N; }

  constexpr bool append(char c) noexcept
  {
    if (m_Size >= N) { return false; }
    m_Buffer[m_Size++] = c;
    m_Buffer[m_Size] = '\0';
    return true;
  }

  constexpr bool append(const char* s) noexcept
  {
    while (*s != '\0')
    {
      if (m_Size >= N) { return false; }
      m_Buffer[m_Size++] = *s++;
    }
    m_Buffer[m_Size] = '\0';
    return true;
  }

  constexpr bool append(std::string_view s) noexcept
  {
    for (char c : s)
    {
      if (m_Size >= N) { return false; }
      m_Buffer[m_Size++] = c;
    }
    m_Buffer[m_Size] = '\0';
    return true;
  }

private:
  char m_Buffer[N + 1]{};
  std::size_t m_Size{};
};

using string4_t = fixed_string<4>;
using string8_t = fixed_string<8>;
using string16_t = fixed_string<16>;
using string32_t = fixed_string<32>;
using string64_t = fixed_string<64>;

}

#endif