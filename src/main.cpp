#include "app_refactor.hpp"
#include <Arduino.h>

void setup() {
  // Serial.begin(115200);
  // LOG("looping");
  sndbx::app::init();
  // App::init();
}

void loop() {
  // LOG("looping");
  // app.loop();
  sndbx::app::loop();
}