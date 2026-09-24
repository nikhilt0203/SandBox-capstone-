#include <Arduino.h>
#include <nst/hardware/ILI9341_display.hpp>

constexpr auto tft_cs = nst::Pin{14};
constexpr auto tft_dc = nst::Pin{15};

// Initialize TFT with CS and DC pin
nst::teensy::TFT screen{tft_cs, tft_dc};

std::uint32_t last_render_time{};
bool swap_text{false};

void setup() {
  screen.clear();
  screen.set_frame_available();
}

void loop() {
  const auto now = millis();
  //Display every 100ms
  if (now - last_render_time >= 100) {
    if (swap_text) {
      //ScreenElements should be immediately drawn after construction
      nst::teensy::Text{50, 50, "hello", 0xFF00, 4, screen.current_frame()}.draw();
    } else {
      nst::teensy::Text{50, 50, "world", 0x00FF, 4, screen.current_frame()}.draw();
    }
    swap_text = !swap_text;
    // Tell screen to render on the next call to render_frame
    screen.set_frame_available();
    last_render_time = now;
  }
  // Render the frame onto the screen
  screen.render_frame();
}