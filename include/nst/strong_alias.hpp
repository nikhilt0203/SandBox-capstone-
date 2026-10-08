#ifndef NST_STRONG_ALIAS_HPP_
#define NST_STRONG_ALIAS_HPP_

#include <type_traits>

namespace nst {

/// @brief  A strong alias for a trivial type
/// @tparam T the aliased type
/// @tparam Tag unique tag
template <typename T, typename Tag> struct strong_alias {
	static_assert(std::is_trivial_v<T>, "Aliased type must be trivial.");

	using underlying_t = T;
	using tag_type = Tag;

	T value{};

	constexpr strong_alias() = default;
	constexpr explicit strong_alias(T value) : value{value} {}
	constexpr explicit operator T() const { return value; }
};

template <typename AliasType>
[[nodiscard]] constexpr auto to_underlying_t(AliasType alias) {
	return static_cast<typename AliasType::underlying_t>(alias);
}

} // namespace nst

#endif