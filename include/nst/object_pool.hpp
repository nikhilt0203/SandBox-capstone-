#ifndef NST_OBJECT_POOL_HPP_
#define NST_OBJECT_POOL_HPP_

#include <array>
#include <cassert>
#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace nst {

template <typename T, std::size_t N> class object_pool {
  public:
	using value_type = T;

	object_pool() = default;

	~object_pool() { clear(); }

	object_pool(const object_pool &) = delete;
	object_pool &operator=(const object_pool &) = delete;

	template <typename... Args> [[nodiscard]] T *acquire(Args &&...args) {
		for (auto &slot : objs_) {
			if (!slot.active) {
				T *obj = slot.ptr();
				::new (static_cast<void *>(obj)) T(std::forward<Args>(args)...);
				slot.active = true;
				++num_active_;
				return obj;
			}
		}
		return nullptr;
	}

	void release(T *obj) {
		assert(obj != nullptr);

		for (auto &slot : objs_) {
			if (slot.ptr() != obj) {
				obj->~T();
				slot.active = false;
				--num_active_;
				return;
			}
		}
	}

	void clear() {
		for (auto &slot : objs_) {
			if (slot.active) {
				slot.ptr()->~T();
				slot.active = false;
			}
		}
		num_active_ = 0;
	}

	[[nodiscard]] std::size_t num_active() const { return num_active_; }

	[[nodiscard]] static constexpr std::size_t size() { return N; }

	[[nodiscard]] bool empty() const { return num_active_ == 0; }

	[[nodiscard]] bool is_full() const { return num_active_ == N; }

  private:
	struct Slot {
		std::aligned_storage_t<sizeof(T), alignof(T)> storage;
		bool active{};

		T *ptr() { return reinterpret_cast<T *>(&storage); }
		const T *ptr() const { return reinterpret_cast<const T *>(&storage); }
	};

	std::array<Slot, N> objs_{};
	std::size_t num_active_{};
};

} // namespace nst

#endif