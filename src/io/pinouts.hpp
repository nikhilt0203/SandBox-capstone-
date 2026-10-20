#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_

#include <array>
#include <tuple>

#include <nst/pin.hpp>

namespace sndbx::pinouts {

inline constexpr std::array encoder_pins{std::pair<nst::Pin, nst::Pin>{33, 34},
                                         std::pair<nst::Pin, nst::Pin>{35, 36},
                                         std::pair<nst::Pin, nst::Pin>{37, 38},
                                         std::pair<nst::Pin, nst::Pin>{39, 40}};

inline constexpr std::array button_pins{nst::Pin{32}, nst::Pin{30},
                                        nst::Pin{31}, nst::Pin{29}};

inline constexpr std::array trellis_addrs{0x2E, 0x30, 0x31, 0x2f};

inline constexpr nst::Pin tft_cs{14};
inline constexpr nst::Pin tft_dc{15};

} // namespace sndbx::pinouts

#endif