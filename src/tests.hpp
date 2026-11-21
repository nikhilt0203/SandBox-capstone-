#ifndef SANDBOX_TESTS_HPP_
#define SANDBOX_TESTS_HPP_

#include "modules/oscillator.hpp"
#include <nst/teensy/audio_graph.hpp>

void audio_graph_basic_test() {
	AudioMemory(100);

	::AudioControlSGTL5000 audio_shield;
	::AudioOutputI2S output;

	audio_shield.enable();
	audio_shield.volume(1.0);

	sndbx::AudioGraph graph;
	sndbx::Oscillator osc;

	assert(osc.link(graph));
	const auto out_id = *graph.add(&output);
	const auto [osc_id, _] = osc.patch_point({sndbx::PortType::OUT});

	constexpr static auto ms = 1500;
	while (true) {
		graph.connect(osc_id, 0, out_id, 1);
		delay(ms);
		graph.disconnect(osc_id, out_id);
		delay(ms);
		graph.connect(osc_id, out_id);
		delay(ms);
		graph.disconnect(osc_id, out_id);
		delay(ms);
	}
}

// Abstraction is clear now:
// - AudioGraph manages connection, disconnection, and gives ID-addressability.
// - Patchable modules represent a unit of AudioStream objects that can be
// patched to
//   in the audio graph. The patching points are exposed by the member function
//   patch_point()
// - Trivial to test, it is a bit ugly, but it should be! We're "converting"
// between two levels of abstraction:
//   Module-land to AudioGraph land
//   if we had two Module types. But in my opinion this is a satisfying solution
//   showing the benefit of identifying levels of abstraction and how testing
//   can reveal architectural problems.

void test2() {
	sndbx::AudioGraph graph1;
	sndbx::AudioGraph graph2;
	sndbx::Oscillator osc;

	while (true) {
		osc.link(graph1);

		delay(1500);

		osc.unlink(graph1);
		osc.link(graph2);

		delay(1500);

		osc.unlink(graph2);
	}
}

// Why not? The architectural oundaries are sufficiently managed such that
// flexibility is unlocked
#endif