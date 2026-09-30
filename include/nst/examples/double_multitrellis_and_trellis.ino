#include <nst/hardware/trellis.hpp>
#include <vector>

// ****************************************************************************
// Example with 2 differently sized MultiTrellis-es, 1 single trellis,
// and customization of the key event type.
// ****************************************************************************

using namespace nst::teensy;

// I2C addresses for each NeoTrellis board
constexpr std::array i2c_addrs1{0x2E, 0x30, 0x31, 0x2f};
constexpr std::array i2c_addrs2{0x3F, 0x2D, 0x27, 0x17, 0x3A,
                                0x19, 0x27, 0x18, 0x1F};
constexpr unsigned char i2c_addr3{0x5A};

// Example wrapper type to add a timestamp field
struct MyKeyEvent {
  MyKeyEvent(const TrellisKeyEvent &e) : data(e) {}
  TrellisKeyEvent data;
  unsigned long timestamp{millis()};
};

static std::vector<MyKeyEvent> keypad_events;

// Create MultiTrellis keypad w/ I2C addresses, event queue, and unique ID
static auto keypad1 = make_multitrellis<i2c_addrs1>(keypad_events, 0);
static auto keypad2 = make_multitrellis<i2c_addrs2>(keypad_events, 1);
static auto keypad3 = make_trellis<i2c_addr3>(keypad_events, 2);

// Event handler
void handle_key_event(const MyKeyEvent &e) {
  const auto [edge, key_num, keypad_num] = e.data;

  // Print event info
  switch (edge) {
  case TrellisKeyEvent::Edge::RISING_EDGE:
    Serial.printf("Key %d pressed on keypad %d at %u ms\n", key_num, keypad_num,
                  e.timestamp);
    break;
  case TrellisKeyEvent::Edge::FALLING_EDGE:
    Serial.printf("Key %d released on keypad %d at %u ms\n", key_num,
                  keypad_num, e.timestamp);
    break;
  default:
    break;
  }
}

void setup() { Serial.begin(9600); }

void loop() {
  keypad1.update();
  keypad2.update();
  keypad3.update();

  // All keypads push events into the same queue.
  // Separate queues could also be used
  while (!keypad_events.empty()) {
    auto event = keypad_events.back();
    keypad_events.pop_back();
    handle_key_event(event);
  }
}