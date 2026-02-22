#ifndef parameter_hpp_
#define parameter_hpp_

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
  T& value() { return m_Value; }
  T max() const { return m_Max; }
  T min() const { return m_Min; }

  void change(ChangeFunc changeFunc, int delta) 
  { 
    m_Value = std::clamp(changeFunc(m_Value, delta), m_Min, m_Max);
  }

  float normalized() const 
  { 
    return static_cast<float>(m_Value - m_Min) / static_cast<float>(m_Max - m_Min);
  }
};

#endif