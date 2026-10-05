#ifndef SANDBOX_MODULE_DISPLAY_INFO_HPP_
#define SANDBOX_MODULE_DISPLAY_INFO_HPP_

#include "config/config.hpp"
#include <array>
#include <nst/color.hpp>
#include <nst/inplace_string.hpp>
#include <string_view>
#include <tuple>

namespace sndbx {

using ModulePortName = nst::inplace_string<limits::max_port_name_len>;
using ModuleControlName = nst::inplace_string<limits::max_ctrl_name_len>;

using ModulePortNames = std::array<ModulePortName, limits::max_in_ports>;

using ModuleControlNames =
    std::array<ModuleControlName, limits::max_module_ctrls>;

using ModuleName = std::pair<nst::inplace_string<limits::max_module_name_len>,
                             nst::teensy::ColorRGB>;

using ModuleDescription = nst::inplace_string<limits::max_module_desc_len>;

struct ModuleDisplayInfo {
	ModulePortNames in_names;
	ModulePortNames out_names;
	ModuleControlNames ctrl_names;
	ModuleName name;
	ModuleDescription description;

	constexpr ModuleDisplayInfo(ModuleName name, ModuleDescription desc,
	                            ModulePortNames ins, ModulePortNames outs,
	                            ModuleControlNames ctrls)
	    : in_names{ins}, out_names{outs}, ctrl_names{ctrls}, name{name},
	      description{desc} {}
};

// default display info, specialize template in each module .hpp
template <class Module>
inline constexpr ModuleDisplayInfo module_info = {
    {"unnamed", 0xFFFFFF}, "N/A", {}, {}, {}};

} // namespace sndbx

#endif