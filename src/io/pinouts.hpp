#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_

#include <array>
#include <nst/pin.hpp>

namespace sndbx::pinouts {

inline constexpr auto encoder_pins =
    std::array{std::pair<nst::pin, nst::pin>{33, 34},
               std::pair<nst::pin, nst::pin>{35, 36},
               std::pair<nst::pin, nst::pin>{37, 38},
               std::pair<nst::pin, nst::pin>{39, 40}};

inline constexpr auto button_pins =
    std::array{nst::pin{32}, nst::pin{30}, nst::pin{31}, nst::pin{29}};

inline constexpr auto tft_cs = nst::pin{14};
inline constexpr auto tft_dc = nst::pin{15};

inline constexpr auto trellis_addrs =
    std::array<uint8_t, 4>{0x2E, 0x30, 0x31, 0x2f};

} // namespace sndbx::pinouts

#endif