#ifndef NST_ENCODER_HPP_
#define NST_ENCODER_HPP_

#include "Encoder.h"
#include <array>
#include <nst/pin.hpp>

namespace nst::teensy {

struct EncoderTurnEvent {
	std::uint8_t encoder_num;
	std::int8_t delta;

	constexpr EncoderTurnEvent(std::uint8_t encoder_num, std::int8_t delta)
	    : encoder_num{encoder_num}, delta{delta} {}
};

namespace detail {

template <typename EventContainer>
inline constexpr bool can_hold_encoder_events_v =
    std::is_constructible_v<typename EventContainer::value_type,
                            EncoderTurnEvent>;

} // namespace detail

template <typename EventContainer> class RotaryEncoder {
	static_assert(
	    detail::can_hold_encoder_events_v<EventContainer>,
	    "Container must hold EncoderTurnEvent or another convertible type.");

  public:
	RotaryEncoder(nst::Pin pin1, nst::Pin pin2, std::uint8_t encoder_num,
	              EventContainer &events)
	    : encoder_{static_cast<std::uint8_t>(pin1),
	               static_cast<std::uint8_t>(pin2)},
	      events_{events}, encoder_num_{encoder_num} {}

	void update() {
		const auto cur_pos = read();
		const auto delta = cur_pos - old_pos_;

		const std::int8_t steps = delta / 4;

		if (steps != 0) {
			if constexpr (std::is_same_v<typename EventContainer::value_type,
			                             EncoderTurnEvent>) {
				events_.emplace_back(encoder_num_, steps);
			} else {
				events_.emplace_back(EncoderTurnEvent{encoder_num_, steps});
			}
			old_pos_ += delta;
		}
	}

	[[nodiscard]] auto read() const { return encoder_.read(); }
	[[nodiscard]] std::size_t encoder_num() const { return encoder_num_; }

  private:
	mutable Encoder encoder_;
	EventContainer &events_;
	std::int16_t old_pos_{};
	std::uint8_t encoder_num_;
};

template <std::size_t N>
using EncoderPins = std::array<std::pair<nst::Pin, nst::Pin>, N>;

namespace detail {
template <std::size_t N, typename EventContainer, std::size_t... Is>
auto make_encoder_array_impl(const EncoderPins<N> &encoder_pins,
                             EventContainer &events,
                             std::index_sequence<Is...>) {
	return std::array{RotaryEncoder{encoder_pins[Is].first,
	                                encoder_pins[Is].second, Is, events}...};
}

} // namespace detail

template <std::size_t N, typename EventContainer>
[[nodiscard]] auto make_encoder_array(const EncoderPins<N> &encoder_pins,
                                      EventContainer &events) {
	static_assert(
	    detail::can_hold_encoder_events_v<EventContainer>,
	    "Container must hold EncoderTurnEvent or another convertible type.");
	return detail::make_encoder_array_impl<N>(encoder_pins, events,
	                                          std::make_index_sequence<N>{});
}

} // namespace nst::teensy

#endif