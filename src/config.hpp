#ifndef SANDBOX_CONFIG_HPP_
#define SANDBOX_CONFIG_HPP_

#include <cstddef>
#include <nst/strong_alias.hpp>

namespace sndbx::limits {

constexpr auto max_modules = 64U;
constexpr auto max_connections = 64U;
constexpr auto max_in_ports = 8U;
constexpr auto max_out_ports = 8U;
constexpr auto max_input_evts = 32U;

} // namespace sndbx::limits

namespace sndbx::config {

constexpr auto trellis_rows = 2U;
constexpr auto trellis_cols = 2U;
constexpr auto num_encoders = 4U;
constexpr auto num_buttons = 4U;
constexpr auto screen_width_px = 320U;
constexpr auto screen_height_px = 240U;
constexpr auto grid_rows = 8U;
constexpr auto grid_cols = 8U;

} // namespace sndbx::config

#endif