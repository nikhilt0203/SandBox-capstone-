#include <nst/hardware/trellis.hpp>
#include <vector>

// ****************************************************************************
// Basic example for 2x2 MultiTrellis
// ****************************************************************************

using namespace nst::teensy;

// std::array of I2C address for each NeoTrellis board (any amount)
constexpr std::array i2c_addrs{0x2E, 0x30, 0x31, 0x2f, 0x2E, 0x30, 0x31, 0x2f};

// Event queue can be any emplace_back-supporting container
//(holding a type constructible from nst::teensy::TrellisKeyEvent)
static std::vector<TrellisKeyEvent> key_events;

// Create MultiTrellis keypad w/ I2C addresses (passed as template param) and
// a reference to the event queue
static auto keypad = make_multitrellis<i2c_addrs>(key_events);

void handle_key_event(const TrellisKeyEvent &e) {
	Serial.printf("Key %d pressed!\n", e.key_num);
}

void setup() { Serial.begin(9600); }

void loop() {
	keypad.update();
	// Empty event queue
	while (!key_events.empty()) {
		const auto event = key_events.back();
		key_events.pop_back();
		handle_key_event(event);
	}
}