#ifndef SANDBOX_ANIMATABLE_HPP_
#define SANDBOX_ANIMATABLE_HPP_

#include "Adafruit_GFX.h"

namespace sndbx {
class Animatable {
  public:
	virtual ~Animatable() = default;
	virtual void draw(GFXcanvas16 &frame) const = 0;
};
} // namespace sndbx
#endif