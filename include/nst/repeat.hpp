#ifndef NST_REPEAT_HPP_
#define NST_REPEAT_HPP_

#include <utility>

namespace nst {
namespace detail {

template <std::size_t... Is, typename Func>
void repeat(Func &&f, std::index_sequence<Is...>) {
	((Is, std::forward<Func>(f)()), ...);
}
} // namespace detail

// Force loop unroll via fold expression
template <std::size_t N, typename Func> void repeat(Func &&f) {
	detail::repeat(std::forward<Func>(f), std::make_index_sequence<N>{});
}

} // namespace nst

#endif