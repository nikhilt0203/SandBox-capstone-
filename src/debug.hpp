#pragma once
#include <Arduino.h>

void debug_print() {
  Serial.println("DEBUG!");
}

#define BREAKPOINT assert(false)