#ifndef SANDBOX_CONFIG_HPP_
#define SANDBOX_CONFIG_HPP_

#include <cstddef>
#include <nst/strong_alias.hpp>

namespace sndbx::limits {

inline constexpr auto max_audio_graph_nodes = 128U;
inline constexpr auto max_audio_graph_patches = 128U;
inline constexpr auto max_modules = 64U;
inline constexpr auto max_module_connections = 128U;
inline constexpr auto max_in_ports = 8U;
inline constexpr auto max_out_ports = 8U;
inline constexpr auto max_module_ctrls = 4U;
inline constexpr auto max_ctrl_name_len = 6U;
inline constexpr auto max_port_name_len = 3U;

inline constexpr auto max_file_line_len = 72U;
inline constexpr auto max_lines_in_file = 128U;
inline constexpr auto max_files_in_directory = 128U;
inline constexpr auto max_filename_len = 32U;
inline constexpr auto max_module_name_len = 13U;
inline constexpr auto max_module_desc_len = 72U;

inline constexpr auto max_input_evts = 32U;
inline constexpr auto screen_width_px = 320U;
inline constexpr auto screen_height_px = 240U;

inline constexpr auto serialization_buffer_max = 256U;

inline constexpr auto grid_rows = 8U;
inline constexpr auto grid_cols = 8U;
inline constexpr auto grid_size = grid_rows * grid_cols;

} // namespace sndbx::limits

namespace sndbx::config {

constexpr auto trellis_rows = 2U;
constexpr auto trellis_cols = 2U;
constexpr auto num_encoders = 4U;
constexpr auto num_buttons = 4U;

} // namespace sndbx::config

#endif