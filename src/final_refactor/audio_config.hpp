#ifndef SANDBOX_AUDIO_CONFIG_HPP_
#define SANDBOX_AUDIO_CONFIG_HPP_

#include <cstdint>

namespace sndbx::config::audio {

constexpr std::size_t max_modules = 64U;
constexpr std::size_t max_connections = 64U;
constexpr std::size_t max_input_ports = 8U;
constexpr std::size_t max_output_ports = 8U;

}

#endif