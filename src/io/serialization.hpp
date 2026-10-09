#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include "config/config.hpp"
#include "modules/module_display_info.hpp"
#include "modules/module_types.hpp"
#include <array>
#include <cassert>

namespace sndbx {

struct SerializationBuffer {
	std::array<std::byte, limits::serialization_buffer_max> data{};

	void write(std::byte *b, std::size_t n) {
		assert(data.size() - size_ >= n);
		for (std::size_t i{}; i < n; ++i) {
			data[size_++] = b[i];
		}
	}

	void write(std::byte b) {
		assert(size_ != data.size());
		data[size_++] = b;
	}

	[[nodiscard]] const auto size() const { return size_; }

  private:
	std::size_t size_{};
};

// Probably don't need this, state lives in systems
class Serializable {
	virtual ~Serializable() = default;
	virtual void serialize_to(SerializationBuffer &) const = 0;
};

} // namespace sndbx
#endif