#ifndef NST_TEENSY_AUDIO_GUARD_HPP_
#define NST_TEENSY_AUDIO_GUARD_HPP_

#include <Audio.h>

namespace nst::teensy {

// RAII wrapper for disabling audio interrupts
struct AudioGuard {
	AudioGuard() { AudioNoInterrupts(); }
	~AudioGuard() { AudioInterrupts(); }

	AudioGuard(const AudioGuard &) = delete;
	AudioGuard(AudioGuard &&) = delete;
	AudioGuard &operator=(const AudioGuard &) = delete;
	AudioGuard &operator=(const AudioGuard &&) = delete;
};

} // namespace nst::teensy
#endif