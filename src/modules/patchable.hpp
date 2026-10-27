#ifndef SANDBOX_PATCHABLE_HPP_
#define SANDBOX_PATCHABLE_HPP_

#include "audio.hpp"
#include <cstdint>

namespace sndbx {

// Interface between a module and the lower-level AudioGraph.
class Patchable {
  public:
	Patchable(std::uint8_t ins, std::uint8_t outs) : ins_{ins}, outs_{outs} {}
	virtual ~Patchable() = default;

	// create the required nodes and/or connections in the audio graph. If any
	// operation fails, undo the previous operations, then return the error
	[[nodiscard]] virtual auto link(AudioGraph &graph) -> AudioError = 0;

	// remove any nodes created in link() from the graph
	virtual void unlink(AudioGraph &graph) = 0;

	// map a module port to an AudioEndpoint
	[[nodiscard]] virtual auto patch_point(ModulePort port) const
	    -> AudioEndpoint = 0;

	[[nodiscard]] std::uint8_t ins() const { return ins_; }
	[[nodiscard]] std::uint8_t outs() const { return outs_; }

  private:
	std::uint8_t ins_;
	std::uint8_t outs_;
};

} // namespace sndbx
#endif