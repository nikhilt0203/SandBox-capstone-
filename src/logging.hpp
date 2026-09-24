#ifndef SANDBOX_LOGGING_HPP_
#define SANDBOX_LOGGING_HPP_

#define DEBUG

#ifdef DEBUG
#define LOG(msg) Serial.println(msg)
#define LOGF(msg) Serial.printf(msg)
#else
#define LOG(msg) 0
#define LOGF(msg) 0
#endif

#endif