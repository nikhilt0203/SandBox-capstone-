#ifndef SANDBOX_MODULE_DISPLAY_INFO_HPP_
#define SANDBOX_MODULE_DISPLAY_INFO_HPP_

#include <array>
#include <charconv>
#include <string_view>
#include <tuple>

#include <nst/color.hpp>
#include <nst/inplace_string.hpp>

#include "config/config.hpp"

namespace sndbx {

using ModulePortName = nst::inplace_string<limits::max_port_name_len>;
using ModuleControlName = nst::inplace_string<limits::max_ctrl_name_len>;

using ModulePortNames = std::array<ModulePortName, limits::max_in_ports>;

using ModuleControlNames =
    std::array<ModuleControlName, limits::max_module_ctrls>;
    
using ModuleName = std::pair<nst::inplace_string<limits::max_module_name_len>,
                             nst::teensy::ColorRGB>;

using ModuleDescription = nst::inplace_string<limits::max_module_desc_len>;

using CtrlFmtStr = nst::inplace_string<8>;
using ControlFormatters =
    std::array<CtrlFmtStr (*)(float), limits::max_module_ctrls>;

using ModuleControlValues =
    nst::inplace_vector<CtrlFmtStr, limits::max_module_ctrls>;
    
struct ModuleDisplayInfo {
	ModulePortNames in_names;
	ModulePortNames out_names;
	ModuleControlNames ctrl_names;
	ModuleName name;
	ModuleDescription description;
	ControlFormatters ctrl_fmt;

	constexpr ModuleDisplayInfo(ModuleName name, ModuleDescription desc,
	                            ModuleControlNames ctrls, ModulePortNames ins,
	                            ModulePortNames outs,
	                            ControlFormatters ctrl_fmt)
	    : in_names{ins}, out_names{outs}, ctrl_names{ctrls}, name{name},
	      description{desc}, ctrl_fmt{ctrl_fmt} {}
};

template <typename T> inline constexpr CtrlFmtStr to_str(T value) {
	CtrlFmtStr str;
	std::to_chars(str.data(), str.data() + 5, value);
	return str;
}

inline constexpr auto format(float value, std::string_view suffix = "") {
	auto str = to_str(value);
	str.append(suffix);
	return str;
}

template <typename FormatPolicy>
inline constexpr auto format_ctrl(float value, FormatPolicy fp) {
	auto str = to_str(value);
	return str;
}

struct FDefault {};
template <> inline constexpr auto format_ctrl<FDefault>(float value, FDefault) {
	return to_str(value);
}

struct FHertz {};
template <> inline constexpr auto format_ctrl<FHertz>(float value, FHertz) {
	return format(value, " hz");
}

struct FOctaves {};
template <> inline constexpr auto format_ctrl<FOctaves>(float value, FOctaves) {
	return format(value, " 8ve");
}

struct FMillis {};
template <> inline constexpr auto format_ctrl<FMillis>(float value, FMillis) {
	return format(value, " ms");
}

// Formats the control value into a string according to the FormatPolicy
template <typename FormatPolicy = FDefault>
inline constexpr auto ctrl_format(float value) {
	return format_ctrl(value, FormatPolicy{});
}

// Specialize per module
template <class Module> inline constexpr ControlFormatters ctrl_fmt{};

template <class Module>
inline constexpr ModuleDisplayInfo module_info = {
    {"unnamed", 0xFFFFFF}, "unknown", {}, {}, {}, {}};

template <class Module>
inline constexpr auto num_ins = module_info<Module>.in_names.size();

template <class Module>
inline constexpr auto num_outs = module_info<Module>.out_names.size();

template <class Module>
inline constexpr auto num_ctrls = module_info<Module>.ctrl_names.size();

} // namespace sndbx

#endif