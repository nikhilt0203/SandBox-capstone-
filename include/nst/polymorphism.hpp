#ifndef NST_POLYMORPHISM_HPP_
#define NST_POLYMORPHISM_HPP_

#include <cassert>
#include <tuple>
#include <type_traits>
#include <utility>

namespace nst {

namespace detail {

template <typename T>
using remove_cvref_t =
    std::remove_reference_t<std::remove_volatile_t<std::remove_const_t<T>>>;

template <typename T, typename... Ts>
constexpr bool contains_v = (std::is_same_v<T, Ts> || ...);

template <typename T, typename = void> struct is_poly_view : std::false_type {};

template <typename T>
struct is_poly_view<T, std::void_t<typename T::poly_view_tag>>
    : std::true_type {};

template <typename T>
inline constexpr bool is_poly_view_v = is_poly_view<remove_cvref_t<T>>::value;

template <typename T, typename = void> struct is_poly_ref : std::false_type {};

template <typename T>
struct is_poly_ref<T, std::void_t<typename T::poly_ref_tag>> : std::true_type {
};

template <typename T>
inline constexpr bool is_poly_ref_v = is_poly_ref<remove_cvref_t<T>>::value;

} // namespace detail

// Non-owning polymorphic view over a derived object. Stores a pointer for each
// listed Base class if the viewed object is derived from that Base class, or
// nullptr if it is not.
template <class... PossibleBases> class poly_view {
public:
  using value_type = std::tuple<PossibleBases *...>;
  using poly_view_tag = void;

  template <class Base>
  static constexpr bool contains_base_v =
      (std::is_same_v<std::remove_cv_t<Base>, PossibleBases> || ...);

  template <class...> friend class poly_view;

  // Construct a poly_view from a Derived ptr
  template <class Derived>
  constexpr explicit poly_view(Derived *obj) noexcept
      : ptrs_{([obj]() {
          if constexpr (std::is_base_of_v<PossibleBases, Derived>) {
            return static_cast<PossibleBases *>(obj);
          } else {
            return nullptr;
          }
        }())...} {}

  // Construct a poly_view with a smaller subset of base types from another
  // poly_view
  template <class... OtherBases>
  constexpr poly_view(const poly_view<OtherBases...> &view_superset) noexcept
      : ptrs_{std::get<PossibleBases *>(view_superset.ptrs_)...} {
    using other = poly_view<OtherBases...>;
    static_assert((other::template contains_base_v<PossibleBases> && ...),
                  "New poly_view must only contain types contained in old"
                  "type");
  }

  /// @brief Check if this poly_view is castable to Base
  /// @tparam Base the base type to check
  template <class Base> [[nodiscard]] constexpr bool holds() const noexcept {
    static_assert(contains_base_v<Base>, "Invalid Base type is not stored.");
    return unchecked_get<Base *>() != nullptr;
  }

  template <class Base>
  [[nodiscard]] constexpr const auto &get() const noexcept {
    return static_cast<const Base &>(*this);
  }

  template <class Base> [[nodiscard]] constexpr auto &get() noexcept {
    return static_cast<Base &>(*this);
  }

  template <class Base>
  [[nodiscard]] constexpr const auto get_if() const noexcept {
    return static_cast<const Base *>(*this);
  }

  template <class Base> [[nodiscard]] constexpr auto get_if() noexcept {
    return static_cast<Base *>(*this);
  }

  template <class Base, typename = std::enable_if_t<!std::is_pointer_v<Base>>>
  [[nodiscard]] explicit constexpr operator Base &() noexcept {
    assert(holds<Base>());
    return *unchecked_get<Base *>();
  }

  template <class Base, typename = std::enable_if_t<!std::is_pointer_v<Base>>>
  [[nodiscard]] explicit constexpr operator const Base &() const noexcept {
    assert(holds<Base>());
    return *unchecked_get<Base *>();
  }

  template <class Base>
  [[nodiscard]] explicit constexpr operator const Base *() const noexcept {
    return unchecked_get<Base *>();
  }

  template <class Base>
  [[nodiscard]] explicit constexpr operator Base *() noexcept {
    return unchecked_get<Base *>();
  }

  constexpr operator bool() const { return (holds<PossibleBases>() || ...); }

private:
  template <class BasePtr> constexpr auto unchecked_get() const noexcept {
    using Base = std::remove_cv_t<std::remove_pointer_t<BasePtr>>;
    static_assert(contains_base_v<Base>, "Invalid Base type is not stored.");
    return std::get<Base *>(ptrs_);
  }

  value_type ptrs_;

protected:
  void set(value_type ptrs) { ptrs_ = ptrs; }

  value_type take() {
    const auto tmp = ptrs_;
    ((std::get<PossibleBases *>(ptrs_) = nullptr), ...);
    return tmp;
  }
};

// Owning poly_view. Deletes the object through the first valid base pointer.
template <class... PossibleBases>
class unique_poly_view : public poly_view<PossibleBases...> {
  using base_view = poly_view<PossibleBases...>;
  using value_type = typename base_view::value_type;

public:
  using base_view::base_view;

  unique_poly_view() = delete;

  constexpr unique_poly_view(unique_poly_view &&other) noexcept
      : base_view{other} {
    other.take();
  }

  constexpr unique_poly_view &operator=(unique_poly_view &&other) noexcept {
    if (this != &other) {
      this->reset();
      this->set(other.take());
    }
    return *this;
  }

  ~unique_poly_view() { reset(); }

  [[nodiscard]] value_type release() { return this->take(); }

  void reset() noexcept {
    (try_delete<PossibleBases *>() || ...);
    this->set({});
  }

private:
  template <typename BasePtr> bool try_delete() {
    if (auto ptr = static_cast<BasePtr>(*this)) {
      delete ptr;
      return true;
    }
    return false;
  }
};

// Free helper functions

// Check if poly_view holds a valid pointer for each Bases
template <class... Bases, class... PossibleBases>
[[nodiscard]] constexpr bool
holds(const poly_view<PossibleBases...> &view) noexcept {
  return (view.template holds<Bases>() && ...);
}

// Retrieve a std::tuple<const Bases &...>
template <class... Bases, class... PossibleBases>
[[nodiscard]] constexpr auto
poly_deref(const poly_view<PossibleBases...> &view) noexcept {
  assert(holds<Bases...>(view));
  return std::tuple<const Bases &...>{static_cast<const Bases &>(view)...};
}

// Retrieve a std::tuple<Bases &...>
template <class... Bases, class... PossibleBases>
[[nodiscard]] constexpr auto
poly_deref(poly_view<PossibleBases...> &view) noexcept {
  assert(holds<Bases...>(view));
  return std::tuple<Bases &...>{static_cast<Bases &>(view)...};
}

// Call function f if view can be dereferenced as Base
template <class... Bases, class... PossibleBases, typename U>
constexpr void invoke_if(const poly_view<PossibleBases...> &view, U &&f) {
  if (holds<Bases...>(view)) {
    f(static_cast<const Bases &>(view)...);
  }
}

template <class... Bases, class... PossibleBases, typename U>
constexpr void invoke_if(poly_view<PossibleBases...> &view, U &&f) {
  if (holds<Bases...>(view)) {
    f(static_cast<Bases &>(view)...);
  }
}

// Calls function f for each poly_view
template <class PolyViewContainer, typename U, class... Bases>
constexpr void for_each(PolyViewContainer &c, U &&f) {
  for (auto &view : c) {
    if constexpr (sizeof...(Bases) == 0) {
      f(view);
    } else {
      invoke_if<Bases...>(view, std::forward<U>(f));
    }
  }
}

template <class PolyViewContainer, typename U, class... Bases>
constexpr void for_each(const PolyViewContainer &c, U &&f) {
  for (const auto &view : c) {
    if constexpr (sizeof...(Bases) == 0) {
      f(view);
    } else {
      invoke_if<Bases...>(view, std::forward<U>(f));
    }
  }
}

} // namespace nst

#endif