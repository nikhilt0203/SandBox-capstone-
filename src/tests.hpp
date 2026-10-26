#ifndef SANDBOX_TESTS_HPP_
#define SANDBOX_TESTS_HPP_

#include "modules/oscillator.hpp"
#include <cassert>
#include <nst/audio_graph.hpp>

void audio_graph_basic_test() {
	::AudioControlSGTL5000 audio_shield;
	::AudioOutputI2S output;

	using namespace sndbx;
	AudioGraph graph;
	Oscillator osc;

	audio_shield.enable();
	audio_shield.volume(0.5);

	const auto out_id = *graph.add(&output);

	osc.link(graph);
	const auto [osc_id, _] = osc.patch_point({PortType::OUT});

	constexpr static auto ms = 1500;
	while (true) {
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