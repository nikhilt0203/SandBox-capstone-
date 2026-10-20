#ifndef SANDBOX_TESTS_HPP_
#define SANDBOX_TESTS_HPP_

#include <nst/audio_graph.hpp>

void audio_graph_sine_usb_test() {
	using namespace nst::teensy;
	AudioGraph<2, 1> graph;

	auto sine = *graph.emplace_node<AudioSynthWaveformSine>();
	auto usb_out = *graph.emplace_node<AudioOutputUSB>();

	while (true) {
		assert(graph.connect(sine, AudioPort{0}, usb_out, AudioPort{0}) == AudioGraphError::NONE);
		delay(2000);
		graph.disconnect(sine, usb_out);
		delay(2000);
        break;
	}

    graph.remove_node(sine);
}

#endif