#pragma once

#include <nst/audio_graph.hpp>

void audio_graph_sine_usb_test() {
	using namespace nst::teensy;
	AudioGraph<2, 1> graph;

	auto sine = graph.emplace_node<AudioSynthWaveformSine>();
	assert(sine);
	auto usb_out = graph.emplace_node<AudioOutputUSB>();
	assert(usb_out);

	while (true) {
		assert(graph.connect(sine->id, AudioPort{0}, usb_out->id, AudioPort{0}) == AudioGraphError::NONE);
		delay(2000);
		graph.disconnect(sine->id, usb_out->id);
		delay(2000);
	}
}