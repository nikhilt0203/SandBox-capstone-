#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_

#include <array>
#include <nst/pin.hpp>
#include <tuple>

namespace sndbx::pinouts {

inline constexpr std::array encoder_pins{
    std::make_pair(nst::Pin{33}, nst::Pin{34}),
    std::make_pair(nst::Pin{35}, nst::Pin{36}),
    std::make_pair(nst::Pin{37}, nst::Pin{38}),
    std::make_pair(nst::Pin{39}, nst::Pin{40})};

inline constexpr std::array button_pins{nst::Pin{32}, nst::Pin{30},
                                        nst::Pin{31}, nst::Pin{29}};

inline constexpr std::array trellis_addrs{0x2E, 0x30, 0x31, 0x2f};

inline constexpr nst::Pin tft_cs{14};
inline constexpr nst::Pin tft_dc{15};

} // namespace sndbx::pinouts

#endif