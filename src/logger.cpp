#include "logger.hpp"
#include <Arduino.h>

void sndbx_log(const char *msg) { Serial.printf("SandBox: %s", msg); }

void sndbx_log_error(const char *msg) {
	Serial.printf("SandBox Error: %s", msg);
}