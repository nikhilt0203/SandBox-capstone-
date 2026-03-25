#include <Arduino.h>
#include "app_refactor.hpp"

void setup() 
{
  sndbx::app::init();
}

void loop() 
{ 
  sndbx::app::loop();
}