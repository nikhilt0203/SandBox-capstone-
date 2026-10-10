#ifndef SANDBOX_TYPE_ARRAY_HPP_
#define SANDBOX_TYPE_ARRAY_HPP_

#include <tuple>
#include <utility>

namespace nst {

template <typename... Ts> struct type_list {
	using tuple_type = std::tuple<Ts...>;

	template <std::size_t N>
	using get = std::tuple_element_t<N, std::tuple<Ts...>>;

	using index_sequence = std::index_sequence_for<Ts...>;

	// from stack overflow
	template <typename T>
	static constexpr std::size_t index_of = []() {
		static_assert((std::is_same_v<T, Ts> || ...), "Type not found.");
		bool found = false;
		std::size_t index{};
		((!found ? (++index, found = std::is_same_v<T, Ts>) : 0), ...);
		return index - 1;
	}();

	static constexpr std::size_t size = sizeof...(Ts);

	template <typename T>
	static constexpr bool contains_v = (std::is_same_v<T, Ts> || ...);

	template <typename... Us>
	static constexpr bool contains_all_v = (contains_v<Us> && ...);
};

} // namespace nst

#endif