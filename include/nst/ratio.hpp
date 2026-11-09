#ifndef NST_RATIO_HPP_
#define NST_RATIO_HPP_

namespace nst {

// Represents a ratio constant of 2 integers. Not intended for runtime
// use compared to std::ratio. Convertible to float.
template <int N, int D> struct ratio {
	static_assert(D != 0);

	constexpr operator float() const { return value; }

	static constexpr auto value = static_cast<float>(N) / static_cast<float>(D);
	static constexpr auto numer = N;
	static constexpr auto denom = D;
};

} // namespace nst

#endif
