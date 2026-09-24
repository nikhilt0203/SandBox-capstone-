#ifndef NST_OBJECT_POOL_HPP_
#define NST_OBJECT_POOL_HPP_

#include <array>
#include <cstdint>
#include <type_traits>

namespace nst {

template <typename T, std::size_t N> struct object_pool {
  using value_type = T;

  struct entry {
    T object;
    bool active{};
  };

  std::array<entry, N> objects;

  [[nodiscard]] T *acquire() {
    for (auto &entry : objects) {
      if (!entry.active) {
        entry.active = true;
        ++active_count_;
        return &(entry.object);
      }
    }
    return nullptr;
  }

  void release(T *obj) {
    for (auto &entry : objects) {
      if (&(entry.object) == obj) {
        entry.active = false;
        --active_count_;
        return;
      }
    }
  }

  [[nodiscard]] std::size_t num_active() const { return active_count_; }

  [[nodiscard]] static constexpr auto capacity() { return N; }

private:
  std::size_t active_count_{};
};

} // namespace nst

#endif