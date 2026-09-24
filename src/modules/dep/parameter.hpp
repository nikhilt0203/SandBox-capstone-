#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

template <typename T> class ModuleParameter {
  using ChangeFunc = T (*)(T, int);

public:
  ModuleParameter(T defaultVal, T min, T max)
      : value_(defaultVal), min_(min), max_(max), default_(defaultVal) {}

  operator T() const { return value_; }
  [[nodiscard]] T &value() { return value_; }
  [[nodiscard]] const T &value() const { return value_; }

  [[nodiscard]] T max() const { return max_; }
  [[nodiscard]] T min() const { return min_; }

  /**
   * @brief Set the stored value. Clamps to [min, max].
   *
   * @param value The new value.
   */
  void setValue(T value) { value_ = std::clamp(value, min_, max_); }

  /**
   * @brief Apply a function to the currently stored value.
   *
   * @param changeFunc The function to apply.
   * @param delta The magnitude of change.
   */
  void change(ChangeFunc changeFunc, int delta) {
    setValue(changeFunc(value_, delta));
  }

  /**
   * @brief Reset to the default value.
   */
  void reset() { setValue(default_); }

  /**
   * @brief Maps the current value to [0.0, 1.0].
   *
   * @return float The mapped value.
   */
  [[nodiscard]] float normalized() const {
    return static_cast<float>(value_ - min_) / static_cast<float>(max_ - min_);
  }

  /**
   * @brief Maps an input [0.0, 1.0] to [param min, param max]
   *        and sets the internal value to this mapped value.
   *
   * @param normalized A value [0.0, 1.0].
   * @return T The mapped value.
   */
  T denormalize(float normalized) {
    if (normalized < 0.0f || normalized > 1.0f) {
      return T{};
    }
    value_ = min_ + max_ * normalized;
    return value_;
  }

  ModuleParameter &operator=(T value) {
    setValue(value_);
    return *this;
  }

  ModuleParameter &operator+=(T amt) {
    setValue(value_ + amt);
    return *this;
  }

  ModuleParameter &operator-=(T amt) {
    setValue(value_ - amt);
    return *this;
  }

  ModuleParameter &operator*=(T amt) {
    setValue(value_ * amt);
    return *this;
  }

  ModuleParameter &operator/=(T amt) {
    setValue(value_ / amt);
    return *this;
  }

  ModuleParameter &operator%=(T amt) {
    setValue(value_ % amt);
    return *this;
  }

private:
  T value_;
  T min_;
  T max_;
  T default_;
};

#endif