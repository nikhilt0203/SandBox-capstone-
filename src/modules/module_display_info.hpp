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

template <typename... Ts>
[[nodiscard]] inline constexpr auto to_array(Ts... params) noexcept {
	return std::array<float, limits::max_module_ctrls>{params.ratio()...};
}

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

struct DefaultFormat {};
template <>
inline constexpr auto format_ctrl<DefaultFormat>(float value, DefaultFormat) {
	return to_str(value);
}

struct HertzFormat {};
template <>
inline constexpr auto format_ctrl<HertzFormat>(float value, HertzFormat) {
	return format(value, " hz");
}

struct OctavesFormat {};
template <>
inline constexpr auto format_ctrl<OctavesFormat>(float value, OctavesFormat) {
	return format(value, " 8ve");
}

struct MillisFormat {};
template <>
inline constexpr auto format_ctrl<MillisFormat>(float value, MillisFormat) {
	return format(value, " ms");
}

// Formats the control value into a string according to the FormatPolicy
template <typename FormatPolicy = DefaultFormat>
inline constexpr auto ctrl_formatter(float value) {
	return format_ctrl(value, FormatPolicy{});
}

// Specialize per module
template <class Module> inline constexpr ControlFormatters ctrl_fmt{};

template <class Module>
inline constexpr ModuleDisplayInfo module_info = {
    {"unnamed", 0xFFFFFF}, "no information available", {}, {}, {}, {}};

template <class Module>
inline constexpr auto num_ins = module_info<Module>.in_names.size();

template <class Module>
inline constexpr auto num_outs = module_info<Module>.out_names.size();

template <class Module>
inline constexpr auto num_ctrls = module_info<Module>.ctrl_names.size();

} // namespace sndbx

#endif