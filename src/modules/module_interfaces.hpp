#ifndef SANDBOX_MODULE_INTERFACES_HPP_
#define SANDBOX_MODULE_INTERFACES_HPP_

#include "audio/audio_engine.hpp"
#include "modules/controllable.hpp"
#include "modules/displayable.hpp"
#include "modules/pressable.hpp"

#include <nst/type_list.hpp>

namespace sndbx {

using ModuleInterfaces =
    nst::type_list<audio::Patchable, Controllable, Pressable, Displayable>;

} // namespace sndbx

#endif