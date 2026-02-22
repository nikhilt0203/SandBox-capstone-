#ifndef encoders_hpp_
#define encoders_hpp_

#include "Encoder.h"
#include "pinouts.hpp"

class Encoders
{   
  using EncoderCallback = void(*)(std::size_t, int);
public:
  Encoders(EncoderCallback onTurn) 
  : m_TurnCallback(onTurn)
  { 
    for (std::size_t i{}; i < numEncoders; i++)
    {
      m_EncoderPositions.at(i) = m_Encoders.at(i).read();
    }
  }

  void update()
  {
    for (std::size_t i{}; i < numEncoders; i++)
    {
      auto& previousPos = m_EncoderPositions[i];
      const auto currentPos = static_cast<int>(m_Encoders[i].read());
      const auto delta = currentPos - previousPos;
      static constexpr int minChange = 4;
      
      if (abs(delta) >= minChange)
      {
        m_TurnCallback(i, delta / minChange);
        previousPos = currentPos;
      }
    }
  }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return numEncoders; }

private:
  static constexpr std::size_t numEncoders = 4U;

  std::array<Encoder, numEncoders> m_Encoders = { 
    Encoder{ ENC_PIN_1A, ENC_PIN_1B }, 
    Encoder{ ENC_PIN_2A, ENC_PIN_2B },
    Encoder{ ENC_PIN_3A, ENC_PIN_3B }, 
    Encoder{ ENC_PIN_4A, ENC_PIN_4B }
  };

  std::array<int, numEncoders> m_EncoderPositions{};

  EncoderCallback m_TurnCallback;
};

#endif