#include "app/application.hpp"
//#include "tests.hpp"

//***********************************
//  SandBox modular synthesizer
//  Nikhil Talwalkar 2026
//***********************************

void setup() {
	sndbx::app::init();
	//audio_out_test();
}

void loop() {
	sndbx::app::loop();
}
