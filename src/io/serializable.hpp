#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include "config/config.hpp"
#include <array>

namespace sndbx {

struct SerializationBuffer {
	std::array<std::byte, limits::serialization_buffer_max> data{};
	std::size_t size{};
	void write(std::byte b) { data[size++] = b; }
};

class Serializable {
	virtual ~Serializable() = default;
	virtual void serialize_to(SerializationBuffer &) const = 0;
};

} // namespace sndbx

#endif