#ifndef SANDBOX_EVENT_HPP_
#define SANDBOX_EVENT_HPP_

#include <nst/hardware/button.hpp>
#include <nst/hardware/rotary_encoder.hpp>
#include <nst/hardware/trellis.hpp>
#include <nst/inplace_vector.hpp>
#include <variant>

#include "config.hpp"

namespace sndbx {

struct KeypadEvent {
  using Edge = nst::teensy::TrellisKeyEvent::Edge;
  nst::teensy::TrellisKeyEvent data;
  std::uint32_t time;
  KeypadEvent(nst::teensy::TrellisKeyEvent evt) : data{evt}, time{millis()} {}
};

using KnobEvent = nst::teensy::EncoderTurnEvent;
using ButtonEvent = nst::teensy::ButtonEvent;

using InputEvent = std::variant<KeypadEvent, KnobEvent, ButtonEvent>;

using InputEventQueue =
    nst::inplace_vector<InputEvent, sndbx::limits::max_input_evts>;

} // namespace sndbx

#endif