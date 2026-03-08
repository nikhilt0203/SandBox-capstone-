#ifndef SANDBOX_TYPE_ARRAY_HPP_
#define SANDBOX_TYPE_ARRAY_HPP_

#include <tuple>

namespace sndbx
{
  // compile-time type array using std::tuple -
  // https://stackoverflow.com/a/62139716 Posted by Artyer
  template<typename... Types>
  struct type_array
  {
    template<std::size_t N>
    using get = std::tuple_element_t<N, std::tuple<Types...>>;

    static constexpr std::size_t size = sizeof...(Types); 
  };

  namespace impl
  {
    template<typename T, typename type_array>
    struct IndexOfImpl;

    template<typename T, typename... Ts>
    struct IndexOfImpl<T, type_array<Ts...>>
    {
      // https://stackoverflow.com/a/77853226 Posted by ABu
      // find index of element
      static constexpr std::size_t value()
      {
        static_assert((std::is_same_v<T, Ts> || ...), 
          "The given type does not exist within the given type_array.");

        bool found = false;
        std::size_t index{};
        ((!found ? (++index, found = std::is_same_v<T, Ts>) : 0), ...);
        return index - 1; 
      }
    };
  }

  template<typename T, typename type_array>
  [[nodiscard]] constexpr std::size_t indexOf() { return impl::IndexOfImpl<T, type_array>::value(); }
};

#endif