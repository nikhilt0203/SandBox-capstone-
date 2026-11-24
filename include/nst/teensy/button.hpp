#ifndef NST_BUTTON_HPP_
#define NST_BUTTON_HPP_

#include <array>
#include <cstdint>
#include <nst/pin.hpp>
#include <type_traits>

namespace nst::teensy {

struct ButtonEvent {
	enum class Edge : std::uint8_t { PRESSED, RELEASED };
	std::uint8_t button_num;
	Edge edge;

	constexpr ButtonEvent(std::uint8_t button_num, Edge edge)
	    : button_num{button_num}, edge{edge} {}
};

namespace detail {

template <typename EventContainer>
inline constexpr bool can_hold_button_events_v =
    std::is_constructible_v<typename EventContainer::value_type, ButtonEvent>;

} // namespace detail

class Button {
  public:
	constexpr Button(nst::pin pin, std::uint8_t button_num)
	    : pin_{pin}, button_num_{button_num} {
		pin_.pin_mode(INPUT_PULLUP);
	}

	template <class EventQueue> void update(EventQueue &queue) {
		static_assert(
		    detail::can_hold_button_events_v<EventQueue>,
		    "Container must store ButtonEvent or another convertible type.");

		const auto current_state = pin_.read();
		const bool was_pressed = (last_state_ == LOW);
		const bool is_pressed = (current_state == LOW);

		last_state_ = current_state;

		if (!was_pressed && is_pressed) {
			push_event(queue, ButtonEvent::Edge::PRESSED);
		} else if (was_pressed && !is_pressed) {
			push_event(queue, ButtonEvent::Edge::RELEASED);
		}
	}

	[[nodiscard]] constexpr auto button_num() const { return button_num_; }

  private:
	template <class EventQueue>
	void push_event(EventQueue &queue, ButtonEvent::Edge e) {
		if constexpr (std::is_same_v<typename EventQueue::value_type,
		                             ButtonEvent>) {
			queue.emplace_back(button_num_, e);
		} else {
			queue.emplace_back(ButtonEvent{button_num_, e});
		}
	}

	nst::pin pin_;
	std::uint8_t button_num_;
	std::uint8_t last_state_{HIGH};
};

namespace detail {
template <std::size_t N, std::size_t... Is>
auto make_button_array_impl(const std::array<nst::pin, N> &button_pins,
                            std::index_sequence<Is...>) {
	return std::array{Button{button_pins[Is], Is}...};
}

} // namespace detail

template <std::size_t N>
[[nodiscard]] auto
make_button_array(const std::array<nst::pin, N> &button_pins) {
	return detail::make_button_array_impl<N>(button_pins,
	                                         std::make_index_sequence<N>{});
}

} // namespace nst::teensy

#endif