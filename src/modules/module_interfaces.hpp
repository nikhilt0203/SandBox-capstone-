#ifndef SANDBOX_MODULE_INTERFACES_HPP_
#define SANDBOX_MODULE_INTERFACES_HPP_

#include <nst/type_list.hpp>

#include "io/serializable.hpp"
#include "modules/animatable.hpp"
#include "modules/controllable.hpp"
#include "modules/displayable.hpp"
#include "modules/patchable.hpp"
#include "modules/pressable.hpp"

namespace sndbx {
using ModuleInterfaces = nst::type_list<Patchable, Controllable, Pressable,
                                        Displayable, Serializable, Animatable>;
} // namespace sndbx

#endif