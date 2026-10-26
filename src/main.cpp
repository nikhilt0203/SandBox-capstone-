// #include "app/application.hpp"
#include "tests.hpp"

//***********************************
// SandBox modular synthesizer
// Nikhil Talwalkar 2026
//***********************************

// using namespace sndbx;

void setup() {
	AudioMemory(500);

	// audio_shield.enable();
	// audio_shield.volume(0.5);

	// sine.frequency(1000);
	// sine.amplitude(0.5);
	audio_graph_basic_test();
}

void loop() {
	// app::loop();
}
