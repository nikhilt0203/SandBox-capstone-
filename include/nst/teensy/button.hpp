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

template <class EventContainer> class Button {
	static_assert(
	    detail::can_hold_button_events_v<EventContainer>,
	    "Container must store ButtonEvent or another convertible type.");

  public:
	constexpr Button(nst::pin pin, std::uint8_t button_num,
	                 EventContainer &events)
	    : events_{events}, pin_{pin}, button_num_{button_num} {
		pin_.pin_mode(INPUT_PULLUP);
	}

	void update() {
		const auto current_state = pin_.read();
		const bool was_pressed = (last_state_ == LOW);
		const bool is_pressed = (current_state == LOW);

		last_state_ = current_state;

		if (!was_pressed && is_pressed) {
			push_event(ButtonEvent::Edge::PRESSED);
		} else if (was_pressed && !is_pressed) {
			push_event(ButtonEvent::Edge::RELEASED);
		}
	}

	[[nodiscard]] constexpr auto button_num() const { return button_num_; }

  private:
	void push_event(ButtonEvent::Edge e) {
		if constexpr (std::is_same_v<typename EventContainer::value_type,
		                             ButtonEvent>) {
			events_.emplace_back(button_num_, e);
		} else {
			events_.emplace_back(ButtonEvent{button_num_, e});
		}
	}

	EventContainer &events_;
	nst::pin pin_;
	std::uint8_t button_num_;
	std::uint8_t last_state_{HIGH};
};

namespace detail {

template <std::size_t N, typename EventContainer, std::size_t... Is>
inline constexpr auto
make_button_array_impl(const std::array<nst::pin, N> &pins,
                       EventContainer &events, std::index_sequence<Is...>) {
	return std::array{Button{pins[Is], Is, events}...};
}

} // namespace detail

template <std::size_t N, typename EventContainer,
          typename = std::enable_if_t<
              detail::can_hold_button_events_v<EventContainer>>>
[[nodiscard]] inline constexpr auto
make_button_array(const std::array<nst::pin, N> &pins,
                  EventContainer &event_queue) {
	static_assert(
	    detail::can_hold_button_events_v<EventContainer>,
	    "Container must store ButtonEvent or another convertible type.");
	return detail::make_button_array_impl(pins, event_queue,
	                                      std::make_index_sequence<N>{});
}

} // namespace nst::teensy

#endif