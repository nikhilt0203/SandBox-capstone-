// #ifndef SANDBOX_ENCODERS_HPP_
// #define SANDBOX_ENCODERS_HPP_

// #include "Encoder.h"
// #include "event.hpp"
// #include "config.hpp"
// #include <nst/inplace_vector.hpp>
// #include <type_traits>

// class Encoders {
// public:
//   Encoders() {
//     for (std::size_t i{}; i < numEncoders; ++i) {
//       m_old_positions[i] = m_Encoders[i].read();
//     }
//   }

//   void update() {
//     for (std::size_t i{}; i < numEncoders; ++i) {
//       const auto cur = m_Encoders[i].read();
//       auto &prev = m_old_positions[i];

//       if (const auto delta = cur - prev; abs(delta) >= 4) {
//         m_TurnEvents.emplace_back(i, delta);
//         prev = cur;
//       }
//     }
//   }

//   [[nodiscard]] bool hasEvent() { return !m_TurnEvents.is_empty(); }

//   /**
//    * @brief Pop the last turn event.
//    *
//    * Must check hasEvent() before performing this operation.
//    *
//    * @return sndbx::event::EncoderTurn
//    */
//   [[nodiscard]] sndbx::event::EncoderTurn popEvent() {
//     auto event = m_TurnEvents.back();
//     m_TurnEvents.pop_back();
//     return event;
//   }

//   /**
//    * @brief View the last turn event.
//    *
//    * Must check hasEvent() before performing this operation.
//    *
//    * @return sndbx::event::EncoderTurn
//    */
//   [[nodiscard]] const sndbx::event::EncoderTurn &readEvent() {
//     return m_TurnEvents.back();
//   }

//   [[nodiscard]] constexpr std::size_t size() const { return numEncoders; }

// private:
//   static constexpr std::size_t numEncoders = 4U;

//   std::array<Encoder, numEncoders> m_Encoders = {
//       Encoder{33, 34}, Encoder{35, 36}, Encoder{37, 38}, Encoder{39, 40}};

//   std::array<int, numEncoders> m_old_positions;

//   nst::vector_16U<sndbx::event::EncoderTurn> m_TurnEvents;
// };

// #endif