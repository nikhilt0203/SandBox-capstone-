#include <array>
#include <stdexcept>
#include <initializer_list>

namespace sndbx 
{

template<typename T, std::size_t N>
class fixed_vector
{
public:
  constexpr fixed_vector() = default;

  constexpr fixed_vector(std::initializer_list<T> elements)
  {
    m_Size = elements.size();
    if (m_Size > N) { return; }
    std::size_t index{};
    for (auto& e : elements) { m_Elements[index++] = e; }
  }

  constexpr bool push_back(T element)
  {
    if (m_Size == capacity()) { return false; }
    m_Elements[m_Size] = element;
    m_Size++;
    return true;
  }

  template<typename ...Args>
  constexpr bool emplace_back(Args&&... args)
  {
    if (m_Size == capacity()) { return false; }
    m_Elements[m_Size] = T(std::forward<Args>(args)...);
    m_Size++;
    return true;
  }

  constexpr void pop_back() { if (!is_empty()) { m_Size--; } }

  [[nodiscard]] constexpr bool is_empty() const { return m_Size == 0; }
  [[nodiscard]] constexpr bool is_full() const { return m_Size >= capacity(); }

  constexpr void clear() { m_Size = 0; }

  [[nodiscard]] constexpr const T& operator[](std::size_t index) const { return m_Elements[index]; }
  [[nodiscard]] constexpr T& operator[](std::size_t index) { return m_Elements[index]; }

  [[nodiscard]] const T& at(std::size_t index) const
  {
    if (index >= m_Size) { throw std::out_of_range("Index out of range."); }
    return m_Elements[index];
  }

  [[nodiscard]] T& at(std::size_t index)
  {
    if (index >= m_Size) { throw std::out_of_range("Index out of range."); }
    return m_Elements[index];
  }

  [[nodiscard]] constexpr std::size_t size() const { return m_Size; }
  [[nodiscard]] constexpr std::size_t capacity() const { return N; }

  [[nodiscard]] constexpr T* begin() { return m_Elements.data(); }
  [[nodiscard]] constexpr T* end() { return m_Elements.data() + m_Size; }
  [[nodiscard]] constexpr const T* begin() const { return m_Elements.data(); }
  [[nodiscard]] constexpr const T* end() const { return m_Elements.data() + m_Size; }

private:
  std::size_t m_Size{};
  std::array<T, N> m_Elements{};
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