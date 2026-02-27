#ifndef SANDBOX_PARAMETER_HPP_
#define SANDBOX_PARAMETER_HPP_

template<typename T>
class Parameter
{
using ChangeFunc = T(*)(T, int);
private:
  T m_Value, m_Min, m_Max;
  
public:
  Parameter(T defaultVal, T min, T max) 
  : m_Value(defaultVal), 
    m_Min(min), 
    m_Max(max) 
  {}

  operator T() const { return m_Value; }
  [[nodiscard]] T& value() { return m_Value; }
  [[nodiscard]] const T& value() const { return m_Value; }

  [[nodiscard]] T max() const { return m_Max; }
  [[nodiscard]] T min() const { return m_Min; }

  void setValue(T value) { m_Value = std::clamp(value, m_Min, m_Max); }

  void change(ChangeFunc changeFunc, int delta) { setValue(changeFunc(m_Value, delta)); }

  [[nodiscard]] float normalized() const 
  { 
    return static_cast<float>(m_Value - m_Min) / static_cast<float>(m_Max - m_Min);
  }
};

#endif