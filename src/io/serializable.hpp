#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include "config/config.hpp"
#include "modules/factory/module_types.hpp"
#include <nst/inplace_vector.hpp>

namespace sndbx {

struct SerializationBuffer {
	void write(std::uint16_t b) {
		assert(size_ + 1 < data_.size());
		data_[++size_] = b;
	}
	[[nodiscard]] std::uint16_t read() { return data_[--size_]; }

	const auto &data() const { return data_; }
	auto &data() { return data_; }

  private:
	std::array<std::uint16_t, limits::serialization_buffer_max> data_{};
	std::size_t size_{};
};

class Serializable {
  public:
	virtual ~Serializable() = default;

	virtual void serialize(SerializationBuffer &sb) const = 0;
	virtual void deserialize(SerializationBuffer &sb) = 0;
};

template <typename... Params>
void save_params(SerializationBuffer &sb, const Params &...params) {
	(sb.write(static_cast<std::uint16_t>(
	     normalized(params) *
	     static_cast<float>(std::numeric_limits<std::uint16_t>::max()))),
	 ...);
}

template <typename... Params>
void load_params(SerializationBuffer &sb, Params &...params) {
	(denormalize(params, static_cast<float>(sb.read()) /
	                         static_cast<float>(
	                             std::numeric_limits<std::uint16_t>::max())),
	 ...);
}

} // namespace sndbx

#endif
