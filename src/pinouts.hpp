#ifndef SANDBOX_PINOUTS_HPP_
#define SANDBOX_PINOUTS_HPP_
#include <cstdint>

using pin_t = std::uint8_t;

constexpr pin_t ENC_PIN_1A = 33;
constexpr pin_t ENC_PIN_1B = 34;
constexpr pin_t ENC_PIN_2A = 35;
constexpr pin_t ENC_PIN_2B = 36;
constexpr pin_t ENC_PIN_3A = 37;
constexpr pin_t ENC_PIN_3B = 38;
constexpr pin_t ENC_PIN_4A = 39;
constexpr pin_t ENC_PIN_4B = 40;

constexpr pin_t TRELLIS_1_ADDR = 0x2E;
constexpr pin_t TRELLIS_2_ADDR = 0x30;
constexpr pin_t TRELLIS_3_ADDR = 0x31;
constexpr pin_t TRELLIS_4_ADDR = 0x2F;

constexpr pin_t TFT_CS_PIN = 14;
constexpr pin_t TFT_DC_PIN = 15;

#endif