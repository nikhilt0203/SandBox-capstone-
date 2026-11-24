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

template <class EventContainer>
inline constexpr bool can_hold_encoder_events_v =
    std::is_constructible_v<typename EventContainer::value_type,
                            EncoderTurnEvent>;

} // namespace detail

class RotaryEncoder {
  public:
	RotaryEncoder(nst::pin pin1, nst::pin pin2, std::uint8_t encoder_num)
	    : encoder_{pin1.num, pin2.num}, encoder_num_{encoder_num} {}

	template <class EventQueue> void update(EventQueue &queue) {
		static_assert(detail::can_hold_encoder_events_v<EventQueue>,
		              "Container must hold EncoderTurnEvent or another "
		              "convertible type.");
		const int cur_pos = read();
		const auto delta = cur_pos - old_pos_;

		const std::int8_t steps = delta / 4;

		if (steps != 0) {
			if constexpr (std::is_same_v<typename EventQueue::value_type,
			                             EncoderTurnEvent>) {
				queue.emplace_back(encoder_num_, steps);
			} else {
				queue.emplace_back(EncoderTurnEvent{encoder_num_, steps});
			}
			old_pos_ += delta;
		}
	}

	[[nodiscard]] auto read() const { return encoder_.read(); }
	[[nodiscard]] std::size_t encoder_num() const { return encoder_num_; }

  private:
	mutable Encoder encoder_;
	std::int16_t old_pos_{};
	std::uint8_t encoder_num_;
};

template <std::size_t N>
using EncoderPins = std::array<std::pair<nst::pin, nst::pin>, N>;

namespace detail {
template <std::size_t N, std::size_t... Is>
auto make_encoder_array_impl(const EncoderPins<N> &encoder_pins,
                             std::index_sequence<Is...>) {
	return std::array{
	    RotaryEncoder{encoder_pins[Is].first, encoder_pins[Is].second, Is}...};
}

} // namespace detail

template <std::size_t N>
[[nodiscard]] auto make_encoder_array(const EncoderPins<N> &encoder_pins) {
	return detail::make_encoder_array_impl<N>(encoder_pins,
	                                          std::make_index_sequence<N>{});
}

} // namespace nst::teensy

#endif