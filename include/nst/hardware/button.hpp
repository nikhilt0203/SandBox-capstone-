#ifndef NST_BUTTON_HPP_
#define NST_BUTTON_HPP_

#include <array>
#include <cstdint>
#include <type_traits>

#include <nst/pin.hpp>

namespace nst::teensy {

struct ButtonEvent {
	enum class Edge : std::uint8_t { RISING_EDGE, FALLING_EDGE };
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
	constexpr Button(nst::Pin pin, std::uint8_t button_num,
	                 EventContainer &events)
	    : events_{events}, pin_{pin}, button_num_{button_num} {}

	void update() {
		const auto current_state = pin_.read();
		const bool was_high = (last_state_ == HIGH);
		const bool has_event = was_high ^ (current_state == HIGH);

		last_state_ = current_state;
		if (!has_event) {
			return;
		}

		if constexpr (std::is_same_v<typename EventContainer::value_type,
		                             ButtonEvent>) {
			events_.emplace_back(button_num_,
			                     was_high ? ButtonEvent::Edge::RISING_EDGE
			                              : ButtonEvent::Edge::FALLING_EDGE);
		} else {
			events_.emplace_back(ButtonEvent{
			    button_num_, was_high ? ButtonEvent::Edge::RISING_EDGE
			                          : ButtonEvent::Edge::FALLING_EDGE});
		}
	}

	[[nodiscard]] constexpr auto button_num() const { return button_num_; }

  private:
	EventContainer &events_;
	nst::Pin pin_;
	std::size_t button_num_;
	std::uint8_t last_state_{pin_.read()};
};

namespace detail {

template <std::size_t N, typename EventContainer, std::size_t... Is>
inline constexpr auto
make_button_array_impl(const std::array<nst::Pin, N> &pins,
                       EventContainer &events, std::index_sequence<Is...>) {
	return std::array{Button{pins[Is], Is, events}...};
}

} // namespace detail

template <std::size_t N, typename EventContainer,
          typename = std::enable_if_t<
              detail::can_hold_button_events_v<EventContainer>>>
[[nodiscard]] inline constexpr auto
make_button_array(const std::array<nst::Pin, N> &pins,
                  EventContainer &event_queue) {
	static_assert(
	    detail::can_hold_button_events_v<EventContainer>,
	    "Container must store ButtonEvent or another convertible type.");
	return detail::make_button_array_impl(pins, event_queue,
	                                      std::make_index_sequence<N>{});
}

} // namespace nst::teensy

#endif