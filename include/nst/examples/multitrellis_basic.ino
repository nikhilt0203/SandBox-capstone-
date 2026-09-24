#include <nst/hardware/trellis.hpp>
#include <vector>

// ****************************************************************************
// Basic example for 2x2 MultiTrellis
// ****************************************************************************


// std::array of I2C address for each NeoTrellis board (any amount)
constexpr std::array i2c_addrs{0x2E, 0x30, 0x31, 0x2f, 0x2E, 0x30, 0x31, 0x2f};

// Event queue can be any emplace_back-supporting container
//(holding a type constructible from nst::teensy::TrellisKeyEvent)
using KeypadEvent = nst::teensy::TrellisKeyEvent;
static std::vector<KeypadEvent> keypad_events;

// Create MultiTrellis keypad w/ I2C addresses (passed as template param) and
// a reference to the event queue
static auto keypad = nst::teensy::make_multitrellis<i2c_addrs>(keypad_events);


void handle_key_event(const KeypadEvent &e) {
  Serial.printf("Key %d pressed\n", e.key_num);
}

void setup() { Serial.begin(9600); }

void loop() {
  keypad.update();
  // Check if there are events in the queue, consume them if so
  while (!keypad_events.empty()) {
    const auto event = keypad_events.back();
    keypad_events.pop_back();
    handle_key_event(event);
  }
}