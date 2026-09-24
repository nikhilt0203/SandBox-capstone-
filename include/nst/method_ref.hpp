#ifndef SANDBOX_METHOD_REF_HPP_
#define SANDBOX_METHOD_REF_HPP_

namespace nst {

class method_ref {
public:
  template <typename T, void (T::*method)()>
  constexpr static method_ref bind(T *obj) {
    method_ref callable;
    callable.capture_ = obj;
    callable.method_ = &function<T, method>;
    return callable;
  }

  constexpr void operator()() const { method_(capture_); }

  constexpr operator bool() const { return method_ != nullptr; }

private:
  void *capture_{};
  void (*method_)(void *){};

  template <typename T, void (T::*method)()> static void function(void *obj) {
    (static_cast<T *>(obj)->*method)();
  }
};

} // namespace nst

#endif