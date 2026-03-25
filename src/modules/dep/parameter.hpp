#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

template<typename T>
class ModuleParameter
{
using ChangeFunc = T (*)(T, int);

public:
  ModuleParameter(T defaultVal, T min, T max) 
  : m_Value(defaultVal), 
    m_Min(min), 
    m_Max(max),
    m_Default(defaultVal)
  {}

  operator T() const { return m_Value; }
  [[nodiscard]] T& value() { return m_Value; }
  [[nodiscard]] const T& value() const { return m_Value; }

  [[nodiscard]] T max() const { return m_Max; }
  [[nodiscard]] T min() const { return m_Min; }

  /**
   * @brief Set the stored value. Clamps to [min, max].
   * 
   * @param value The new value.
   */
  void setValue(T value) { m_Value = std::clamp(value, m_Min, m_Max); }

  /**
   * @brief Apply a function to the currently stored value.
   * 
   * @param changeFunc The function to apply.
   * @param delta The magnitude of change.
   */
  void change(ChangeFunc changeFunc, int delta) { setValue(changeFunc(m_Value, delta)); }

  /**
   * @brief Reset to the default value.
   */
  void reset() { setValue(m_Default); }

  /**
   * @brief Maps the current value to [0.0, 1.0].
   * 
   * @return float The mapped value.
   */
  [[nodiscard]] float normalized() const
  { 
    return static_cast<float>(m_Value - m_Min) / static_cast<float>(m_Max - m_Min);
  }

  /**
   * @brief Maps an input [0.0, 1.0] to [param min, param max] 
   *        and sets the internal value to this mapped value.
   * 
   * @param normalized A value [0.0, 1.0].
   * @return T The mapped value.
   */
  T denormalize(float normalized)
  {
    if (normalized < 0.0f || normalized > 1.0f) { return T{}; }
    m_Value = m_Min + m_Max * normalized;
    return m_Value;
  }

private:
  T m_Value;
  T m_Min;
  T m_Max;
  T m_Default;
};

#endif