#ifndef SANDBOX_EVENT_HPP_
#define SANDBOX_EVENT_HPP_

#include <variant>

#include <nst/inplace_vector.hpp>
#include <nst/teensy/button.hpp>
#include <nst/teensy/rotary_encoder.hpp>
#include <nst/teensy/trellis.hpp>

#include "config/config.hpp"

namespace sndbx {

struct KeypadEvent {
	using State = nst::teensy::TrellisEvent::State;

	nst::teensy::TrellisEvent data;
	std::uint32_t time;
	KeypadEvent(nst::teensy::TrellisEvent evt) : data{evt}, time{millis()} {}
};

using KnobEvent = nst::teensy::EncoderTurnEvent;
using ButtonEvent = nst::teensy::ButtonEvent;

using InputEvent = std::variant<KeypadEvent, KnobEvent, ButtonEvent>;

using InputEventQueue = nst::inplace_vector<InputEvent, limits::max_input_evts>;

} // namespace sndbx

#endif