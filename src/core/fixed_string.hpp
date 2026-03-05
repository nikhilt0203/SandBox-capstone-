#include <cstdint>
#include <string_view>

namespace sndbx
{

template<std::size_t N>
class fixed_string
{
public:
  constexpr fixed_string() = default;

  constexpr fixed_string(const char* text) { copy(text); }

  [[nodiscard]] constexpr std::size_t size() const { return m_Size; }
  [[nodiscard]] constexpr std::size_t capacity() const { return N; }

  [[nodiscard]] constexpr const char* data() const { return &m_Buffer[0]; }

  constexpr void operator+=(const char* text) { copy(text); }
  
  constexpr operator const char*() { return data(); }
  constexpr operator std::string_view() { return data(); }

private:
  void copy(const char* text)
  {
    while (text[0] != '\0' && m_Size < N)
    {
      m_Buffer[m_Size] = *text;
      text++;
      m_Size++;
    }
    m_Buffer[m_Size] = '\0';
  }

private:
  char m_Buffer[N];
  std::size_t m_Size{};
};

using string4_t = fixed_string<4>;
using string16_t = fixed_string<16>;
using string50_t = fixed_string<50>;

}