#ifndef SANDBOX_FIXED_VECTOR_HPP_
#define SANDBOX_FIXED_VECTOR_HPP_

#include <array>
#include <initializer_list>
#include <type_traits>
#include <cstring>

// Array wrapper to get vector-like behavior with no reallocations (capacity is fixed)
// constexpr capable
namespace sndbx 
{

template<typename T, std::size_t N>
class fixed_vector
{
public:
  using iterator = T*;
  using const_iterator = const T*;  

  fixed_vector() = default;

  constexpr fixed_vector(std::initializer_list<T> elements)
  {
    m_Size = elements.size();
    std::size_t index{};
    for (auto& e : elements) { m_Elements.at(index++) = e; }
  }

  constexpr bool push_back(const T& element)
  {
    if (m_Size >= N) { return false; }
    m_Elements[m_Size++] = element;
    return true;
  }

  template<typename ...Args>
  constexpr bool emplace_back(Args&&... args)
  {
    if (m_Size >= N) { return false; }
    m_Elements[m_Size++] = T(std::forward<Args>(args)...);
    return true;
  }

  constexpr T* erase(T* pos)
  {
    if (pos < begin() || pos >= end()) { return end(); }

    std::size_t eraseIdx = pos - begin();
  
    if (eraseIdx >= m_Size) { return end(); }

    if constexpr (std::is_trivially_copyable_v<T>)
    {
      std::memmove(&m_Elements[eraseIdx], 
        &m_Elements[eraseIdx + 1], 
        ((m_Size - 1) - eraseIdx) * sizeof(T));
    }
    else
    {
      for (std::size_t i{eraseIdx}; i < m_Size - 1; ++i)
      {
        m_Elements[i] = std::move(m_Elements[i + 1]);
      }
    }

    m_Size--;
    return begin() + eraseIdx;
  }

  constexpr void pop_back() noexcept { if (!is_empty()) m_Size--; }

  constexpr void clear() { m_Size = 0; }

  [[nodiscard]] constexpr bool is_empty() const noexcept { return m_Size == 0; }
  [[nodiscard]] constexpr bool is_full() const noexcept { return m_Size >= capacity(); }

  [[nodiscard]] const T& at(std::size_t index) const { return m_Elements.at(index); }
  [[nodiscard]] T& at(std::size_t index) { return m_Elements.at(index); }

  [[nodiscard]] constexpr const T& operator[](std::size_t index) const noexcept { return m_Elements[index]; }
  [[nodiscard]] constexpr T& operator[](std::size_t index) noexcept { return m_Elements[index]; }

  [[nodiscard]] constexpr const T& back() const noexcept { return m_Elements[m_Size - 1]; }
  [[nodiscard]] constexpr T& back() noexcept { return m_Elements[m_Size - 1]; }

  [[nodiscard]] constexpr std::size_t size() const { return m_Size; }
  [[nodiscard]] constexpr std::size_t capacity() const { return N; }

  [[nodiscard]] constexpr T* begin() { return m_Elements.data(); }
  [[nodiscard]] constexpr T* end() { return m_Elements.data() + m_Size; }
  [[nodiscard]] constexpr const T* begin() const { return m_Elements.data(); }
  [[nodiscard]] constexpr const T* end() const { return m_Elements.data() + m_Size; }

private:
  std::size_t m_Size{};
  std::array<T, N> m_Elements;
};

template<typename T>
using vector_4U = fixed_vector<T, 4>;

template<typename T>
using vector_8U = fixed_vector<T, 8>;

template<typename T>
using vector_16U = fixed_vector<T, 16>;

template<typename T>
using vector_32U = fixed_vector<T, 32>;

template<typename T>
using vector_64U = fixed_vector<T, 64>;

template<typename T>
using vector_128U = fixed_vector<T, 128>;
}

#endif