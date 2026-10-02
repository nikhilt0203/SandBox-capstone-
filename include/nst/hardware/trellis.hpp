#ifndef NST_ADAFRUIT_TRELLIS__HPP_
#define NST_ADAFRUIT_TRELLIS__HPP_

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "Adafruit_NeoTrellis.h"

/// @brief Adafruit NeoTrellis/MultiTrellis wrappers for use with generic event
/// queues. As the Adafruit APIs take function pointer callbacks, these wrapper
/// classes must effectively be be singletons. The I2C address as an NTTP causes
/// a separate type to be instantiated for each address.

/// Trellis/MultiTrellis must be instantiated with a container that supports
/// emplace_back and has a value_type that is constructible from a
/// TrellisKeyEvent. This makes it so that a simple wrapper struct
// can be written to opt into additional event fields (a timestamp, for example)

namespace nst::teensy {

// The type that the Trellis/MultiTrellis pushes into the event queue.
struct TrellisKeyEvent {
	enum class Edge { RISING_EDGE, FALLING_EDGE } edge;
	std::uint16_t key_num;
	std::uint8_t board_num;

	constexpr TrellisKeyEvent(Edge edge, std::uint16_t key_num,
	                          std::uint8_t board_num)
	    : edge(edge), key_num(key_num), board_num(board_num) {}
};

namespace detail {

template <typename EventContainer>
constexpr bool can_hold_key_events_v =
    std::is_constructible_v<typename EventContainer::value_type,
                            TrellisKeyEvent>;

template <typename EventContainer>
inline void push_event(EventContainer &events, ::keyEvent e,
                       std::uint8_t board_num) {
	const auto &[edge, key_num] = e.bit;
	const auto key_edge = (edge == SEESAW_KEYPAD_EDGE_RISING)
	                          ? TrellisKeyEvent::Edge::RISING_EDGE
	                          : TrellisKeyEvent::Edge::FALLING_EDGE;
	// no need to create temporary obj if container holds TrellisKeyEvent
	if constexpr (std::is_same_v<typename EventContainer::value_type,
	                             TrellisKeyEvent>) {
		events.emplace_back(key_edge, key_num, board_num);
	} else {
		events.emplace_back(TrellisKeyEvent{key_edge, key_num, board_num});
	}
}

template <typename Trellis>
[[nodiscard]] inline bool activate_trellis(Trellis &trellis,
                                           TrellisCallback key_callback,
                                           std::size_t num_keys = 16U) {
	if (!trellis.begin()) {
		return false;
	}
	for (std::size_t i{}; i < num_keys; ++i) {
		trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING, true);
		trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING, true);
		trellis.registerCallback(i, key_callback);
		delay(5);
	}
	return true;
}

} // namespace detail

/// @brief Adafruit NeoTrellis wrapper that pushes key events into an external
///        queue.
/// @tparam EventContainer external container type
/// @tparam I2CAddress I2C address of the neotrellis board
template <std::uint8_t I2CAddress, typename EventContainer> class Trellis {
	static_assert(
	    detail::can_hold_key_events_v<EventContainer>,
	    "Container must store TrellisKeyEvent or another convertible type.");

  public:
	/// @brief Trellis constructor
	/// @param events Reference to the external event queue.
	/// @param board_num   Identifier. Only needed with multiple trellis using
	/// the same queue
	Trellis(EventContainer &events, std::uint8_t board_num = 0)
	    : events_{events}, board_num_{board_num} {
		if (!detail::activate_trellis(trellis_, key_callback)) {
			Serial.println("Error: failed to start trellis");
			return;
		}
		assert(instance_ == nullptr && "A trellis with this I2CAddress was "
		                               "already previously instantiated.");
		instance_ = this;
	}

	void update() { trellis_.read(); }

	[[nodiscard]] const auto &neotrellis() const { return trellis_; }
	[[nodiscard]] auto &neotrellis() { return trellis_; }

  private:
	static void key_callback(::keyEvent e) {
		detail::push_event(instance_->events_, e, instance_->board_num_);
	}

  private:
	static inline Trellis *instance_{};

	Adafruit_NeoTrellis trellis_{I2CAddress};
	EventContainer &events_;
	std::uint8_t board_num_;
};

/// @brief Adafruit MultiTrellis wrapper that pushes key events into an external
///        queue.
/// @tparam EventContainer external container type
/// @tparam I2CAddresses compile-time constant array of I2C addresses for each
///         trellis
template <auto &I2CAddresses, typename EventContainer> class MultiTrellis {
	static_assert(
	    detail::can_hold_key_events_v<EventContainer>,
	    "Container must store TrellisKeyEvent or another convertible type.");

  public:
	using ContainerType = EventContainer;

	/// @brief MultiTrellis constructor
	/// @param events Reference to the external event queue.
	/// @param board_num   Identifier. Only needed with multiple objects using
	/// the same queue
	MultiTrellis(EventContainer &events, std::uint8_t board_num = 0)
	    : events_{events}, board_num_{board_num} {
		if (!detail::activate_trellis(multitrellis_, key_callback,
		                              num_keys())) {
			Serial.println("Error: failed to start trellis");
			return;
		}
		assert(instance_ == nullptr && "A multitrellis with these I2CAddresses "
		                               "was already previously instantiated.");
		instance_ = this;
	}

	void update() { multitrellis_.read(); }

	[[nodiscard]] const auto &multitrellis() const { return multitrellis_; }
	[[nodiscard]] auto &multitrellis() { return multitrellis_; }

	[[nodiscard]] static constexpr auto num_keys() { return num_trelli * 16; }

  private:
	static inline MultiTrellis *instance_{};

	static constexpr std::size_t num_trelli = I2CAddresses.size();
	using TrellisArray = std::array<Adafruit_NeoTrellis, num_trelli>;

	template <std::size_t... Is>
	static auto make_trellis_array(std::index_sequence<Is...>) {
		return TrellisArray{Adafruit_NeoTrellis{I2CAddresses[Is]}...};
	}

	static void key_callback(::keyEvent e) {
		detail::push_event(instance_->events_, e, instance_->board_num_);
	}

	TrellisArray trelli_{
	    make_trellis_array(std::make_index_sequence<num_trelli>{})};

	Adafruit_MultiTrellis multitrellis_{trelli_.data(), 1, num_trelli};
	EventContainer &events_;
	std::uint8_t board_num_;
};

/// @brief Creates a MultiTrellis object.
/// @tparam Queue container type
/// @tparam I2CAddresses compile-time constant array of I2C addresses for each
///         neotrellis board
/// @param events external container to hold key events
/// @param board_num unique id for this object
/// @return MultiTrellis
template <auto &I2CAddresses, typename EventContainer>
[[nodiscard]] inline auto make_multitrellis(EventContainer &events,
                                            std::uint8_t board_num = 0) {
	static_assert(
	    detail::can_hold_key_events_v<EventContainer>,
	    "Container must store TrellisKeyEvent or another convertible type.");
	return MultiTrellis<I2CAddresses, EventContainer>{events, board_num};
}

/// @brief Creates a Trellis object.
/// @tparam Queue container type
/// @tparam I2CAddress the I2C address to use
/// @param events external container to hold key events
/// @param board_num unique id for this object
/// @return Trellis
template <std::uint8_t I2CAddress, typename EventContainer>
[[nodiscard]] inline auto make_trellis(EventContainer &events,
                                       std::uint8_t board_num = 0) {
	static_assert(
	    detail::can_hold_key_events_v<EventContainer>,
	    "Container must store TrellisKeyEvent or another convertible type.");
	return Trellis<I2CAddress, EventContainer>{events, board_num};
}

} // namespace nst::teensy

#endif