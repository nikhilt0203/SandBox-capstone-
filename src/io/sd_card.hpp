#ifndef SANDBOX_SD_CARD_HPP_
#define SANDBOX_SD_CARD_HPP_

#include "logger.hpp"
#include <SD.h>

namespace sndbx::sd_card {

[[nodiscard]] inline bool init() {
	if (!SD.begin(BUILTIN_SDCARD)) {
		return false;
	}
	return true;
}

} // namespace sndbx::sd_card

#endif