#include <nst/unreachable.hpp>

namespace nst {
void unreachable() {
	using namespace detail;
	unreachable(assert_on_reach{});
}
} // namespace nst