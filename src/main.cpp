#include <Arduino.h>
#include "app.hpp"

void setup()
{
  sndbx::app::init();
}

void loop()
{
  sndbx::app::loop();
}