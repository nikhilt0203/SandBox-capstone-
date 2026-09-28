#ifndef NST_INPLACE_STRING_HPP_
#define NST_INPLACE_STRING_HPP_

#include <cstdint>
#include <string_view>

namespace nst {

// String with fixed-size inplace storage
template <std::size_t N> class inplace_string {
public:
  constexpr inplace_string() noexcept = default;

  template <std::size_t Size>
  constexpr inplace_string(const char (&s)[Size]) noexcept {
    static_assert(Size - 1 <= N);
    for (std::size_t i{}; i < Size; ++i) {
      buffer_[i] = s[i];
    }
    size_ = Size - 1;
  }

  constexpr explicit inplace_string(const char *s) noexcept { append(s); }

  constexpr explicit inplace_string(std::string_view s) noexcept { append(s); }

  [[nodiscard]] constexpr const char *data() const noexcept { return buffer_; }

  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return std::string_view(data(), size_);
  }

  [[nodiscard]] constexpr char operator[](std::size_t index) const noexcept {
    return buffer_[index];
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
  [[nodiscard]] static constexpr std::size_t capacity() noexcept { return N; }
  [[nodiscard]] constexpr bool is_empty() const noexcept { return size_ == 0; }

  constexpr bool append(char c) noexcept {
    if (size_ >= N) {
      return false;
    }
    buffer_[size_++] = c;
    buffer_[size_] = '\0';
    return true;
  }

  constexpr bool append(const char *s) noexcept {
    while (*s != '\0') {
      if (size_ >= N) {
        return false;
      }
      buffer_[size_++] = *s++;
    }
    buffer_[size_] = '\0';
    return true;
  }

  constexpr bool append(std::string_view s) noexcept {
    for (char c : s) {
      if (size_ >= N) {
        return false;
      }
      buffer_[size_++] = c;
    }
    buffer_[size_] = '\0';
    return true;
  }

private:
  char buffer_[N + 1]{};
  std::size_t size_{};
};

using string4_t = inplace_string<4>;
using string8_t = inplace_string<8>;
using string16_t = inplace_string<16>;
using string32_t = inplace_string<32>;
using string64_t = inplace_string<64>;
using string128_t = inplace_string<128>;
} // namespace nst

#endif