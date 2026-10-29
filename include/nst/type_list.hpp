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

	static constexpr std::size_t size = sizeof...(Ts);

	template <typename T>
	static constexpr bool contains_v = (std::is_same_v<T, Ts> || ...);

	template <typename... Us>
	static constexpr bool contains_all_v = (contains_v<Us> && ...);

	template <typename T>
	static constexpr std::size_t index_of = []() {
		static_assert(type_list::contains_v<T>, "Type not contained.");
		std::size_t index{};
		(((++index, std::is_same_v<T, Ts>)) || ...);
		return index - 1;
	}();
};

} // namespace nst

#endif