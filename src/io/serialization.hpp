#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include <array>
#include <cassert>

#include "config/config.hpp"
#include "modules/factory/module_types.hpp"
#include "modules/module_display_info.hpp"

namespace sndbx {

struct SerializationBuffer
    : public std::array<std::byte, limits::serialization_buffer_max> {
  public:
	using std::array<std::byte, limits::serialization_buffer_max>::array;
	constexpr void push(std::byte *b, std::size_t n) noexcept {
		assert(this->max_size() - size_ >= n);
		for (std::size_t i{}; i < n; ++i) {
			this->operator[](size_++) = b[i];
		}
	}

	constexpr void push(std::byte b) noexcept {
		assert(size_ != this->max_size());
		this->operator[](size_++) = b;
	}

	[[nodiscard]] constexpr auto size() const noexcept { return size_; }

  private:
	std::size_t size_{};
};

// Probably don't need this, state lives in the systems
class Serializable {
	virtual ~Serializable() = default;
	virtual void serialize(SerializationBuffer &sb) const = 0;
};

} // namespace sndbx
#endif