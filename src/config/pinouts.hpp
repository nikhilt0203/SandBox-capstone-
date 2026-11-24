#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_

#include <array>
#include <nst/pin.hpp>

namespace sndbx::pinouts {

using EncoderPins = std::pair<nst::pin, nst::pin>;

inline constexpr auto encoder_pins =
    std::array{EncoderPins{33, 34}, EncoderPins{35, 36}, EncoderPins{37, 38},
               EncoderPins{39, 40}};

inline constexpr auto button_pins = std::array<nst::pin, 4>{32, 30, 31, 29};

inline constexpr auto tft_cs = nst::pin{14};
inline constexpr auto tft_dc = nst::pin{15};

inline constexpr auto trellis_addrs = std::array{0x2E, 0x30, 0x31, 0x2f};

} // namespace sndbx::pinouts

#endif