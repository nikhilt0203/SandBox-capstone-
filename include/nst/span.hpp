#ifndef NST_SPAN_HPP_
#define NST_SPAN_HPP_

namespace nst {

template <typename T> class span {
public:
  using value_type = T;
  using iterator = T *;
  using const_iterator = const T *;

  template <typename Iterator>
  constexpr span(Iterator &&begin, std::size_t size) noexcept
      : data_{&*begin}, size_{size} {}

  template <typename Container,
            typename = std::enable_if_t<!std::is_array_v<Container>>>
  constexpr span(Container &&c) noexcept : span(c.begin(), c.size()) {}

  template <std::size_t N>
  constexpr span(T (&arr)[N]) noexcept : data_{arr}, size_{N} {}

  constexpr span() noexcept = default;

  [[nodiscard]] constexpr const T &operator[](std::size_t index) const {
    assert(index < size_);
    return data_[index];
  }
  [[nodiscard]] constexpr T &operator[](std::size_t index) {
    assert(index < size_);
    return data_[index];
  }

  [[nodiscard]] constexpr const auto begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const auto end() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr const auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr const auto cend() const noexcept { return end(); }

  [[nodiscard]] constexpr auto begin() noexcept { return data_; }
  [[nodiscard]] constexpr auto end() noexcept { return data_ + size_; }

  [[nodiscard]] constexpr const auto &front() const { return *begin(); }
  [[nodiscard]] constexpr const auto &back() const { return *(end() - 1); }
  [[nodiscard]] constexpr auto &front() { return *begin(); }
  [[nodiscard]] constexpr auto &back() { return *(end() - 1); }

  [[nodiscard]] constexpr auto size() const noexcept { return size_; }
  [[nodiscard]] constexpr bool is_empty() const noexcept { return size_ == 0; }

  constexpr T *slide(std::ptrdiff_t amt) {
    data_ += amt;
    return data_;
  }

private:
  T *data_{};
  std::size_t size_{};
};

} // namespace nst

#endif