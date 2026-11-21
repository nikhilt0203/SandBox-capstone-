#ifndef SANDBOX_SERIALIZABLE_HPP_
#define SANDBOX_SERIALIZABLE_HPP_

#include "config/config.hpp"
#include "modules/factory/module_types.hpp"
#include <nst/inplace_vector.hpp>

namespace sndbx {

using SerializationBuffer =
    nst::inplace_vector<std::byte, limits::serialization_buffer_max>;

class Serializable {
  public:
	virtual ~Serializable() = default;

	virtual void serialize(SerializationBuffer &sb) const = 0;
	virtual void deserialize(const SerializationBuffer &sb) = 0;
};

// template <class Module, typename... Params>
// inline void save(SerializationBuffer &sb, Params &&...params) {
// 	sb.push_back(std::byte{module_type<Module>.value}));
// 	(sb.push_back(std::byte{params.()}), ...);
// }

// template <typename... Params>
// inline void load(SerializationBuffer &sb, Params &...params) {
// 	(params.denormalize(sb.pop()), ...);
// }

} // namespace sndbx

#endif
