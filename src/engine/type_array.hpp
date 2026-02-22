#ifndef type_array_hpp_
#define type_array_hpp_

#include <tuple>

namespace sndbx
{
  // compile-time type array using std::tuple -
  // https://stackoverflow.com/a/62139716 Posted by Artyer
  template<typename... Types>
  struct TypeArray
  {
    template<std::size_t N>
    using get = std::tuple_element_t<N, std::tuple<Types...>>;

    static constexpr std::size_t size = sizeof...(Types); 
  };

  namespace impl
  {
    template<typename T, typename TypeArray>
    struct IndexOfImpl;

    template<typename T, typename... Ts>
    struct IndexOfImpl<T, TypeArray<Ts...>>
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

  template<typename T, typename TypeArray>
  [[nodiscard]] constexpr std::size_t indexOf() { return impl::IndexOfImpl<T, TypeArray>::value(); }
};

#endif