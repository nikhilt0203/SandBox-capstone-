#ifndef SANDBOX_MODULE_DISPLAY_INFO_HPP_
#define SANDBOX_MODULE_DISPLAY_INFO_HPP_

#include "config/config.hpp"
#include <array>
#include <nst/color.hpp>
#include <nst/inplace_string.hpp>
#include <string_view>
#include <tuple>

namespace sndbx {

using InputLabels = std::array<nst::inplace_string<limits::max_port_name_len>,
                               limits::max_in_ports>;

using OutputLabels = std::array<nst::inplace_string<limits::max_port_name_len>,
                                limits::max_out_ports>;

using ControlLabels = std::array<nst::inplace_string<limits::max_ctrl_name_len>,
                                 limits::max_module_ctrls>;

using ModuleName = std::pair<nst::inplace_string<limits::max_module_name_len>,
                             nst::teensy::ColorRGB>;

using ModuleDescription = nst::inplace_string<limits::max_module_desc_len>;

struct ModuleDisplayInfo {
	InputLabels in_labels;
	OutputLabels out_labels;
	ControlLabels ctrl_labels;
	ModuleName name;
	ModuleDescription description;

	constexpr ModuleDisplayInfo(ModuleName name, ModuleDescription desc,
	                            InputLabels ins, OutputLabels outs,
	                            ControlLabels ctrls)
	    : in_labels{ins}, out_labels{outs}, ctrl_labels{ctrls}, name{name},
	      description{desc} {}
};

// default display info, specialize template in each module .hpp
template <class Module>
inline constexpr ModuleDisplayInfo module_info{
    {"unnamed", 0xFFFFFF}, "N/A", {}, {}, {}};

} // namespace sndbx

#endif