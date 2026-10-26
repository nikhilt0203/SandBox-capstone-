#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_

#include <array>
#include <tuple>

#include <nst/pin.hpp>

namespace sndbx::pinouts {

inline constexpr auto encoder_pins =
    std::array{std::pair<nst::Pin, nst::Pin>{33, 34},
               std::pair<nst::Pin, nst::Pin>{35, 36},
               std::pair<nst::Pin, nst::Pin>{37, 38},
               std::pair<nst::Pin, nst::Pin>{39, 40}};

inline constexpr auto button_pins =
    std::array{nst::Pin{32}, nst::Pin{30}, nst::Pin{31}, nst::Pin{29}};

inline constexpr auto tft_cs = nst::Pin{14};
inline constexpr auto tft_dc = nst::Pin{15};

inline constexpr auto trellis_addrs =
    std::array<uint8_t, 4>{0x2E, 0x30, 0x31, 0x2f};

} // namespace sndbx::pinouts

#endif