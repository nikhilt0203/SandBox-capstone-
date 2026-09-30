#include <Arduino.h>
#include <nst/hardware/ILI9341_display.hpp>

// Initialize TFT with CS and DC pin
nst::teensy::TFT screen{nst::Pin{14}, nst::Pin{15}};

unsigned long last_render_time = 0;
bool swap_text = false;

void setup() { screen.clear(); }

void loop() {
  const auto now = millis();
  // Display every 100ms
  if (millis() - last_render_time >= 100) {
    if (swap_text) {
      screen.current_frame().println("hello");
    } else {
      screen.current_frame().println("world");
    }
    swap_text = !swap_text;
    last_render_time = now;

    screen.render_frame();
  }
}