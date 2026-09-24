#ifndef SANDBOX_SEQUENCER_MANAGER_HPP_
#define SANDBOX_SEQUENCER_MANAGER_HPP_

#include "engine/module_builder.hpp"
#include "grid.hpp"
#include "modules/sequencer.hpp"

class SequencerManager {
public:
  struct SequencerStepData {
    SequencerStep *step;
    sndbx::grid::Position position;
  };

  struct SequencerData {
    std::uint32_t id{};
    sndbx::grid::Position headPosition{};
    nst::vector_32U<SequencerStepData> steps{};

    SequencerData() = default;

    SequencerData(std::uint32_t id, sndbx::grid::Position position)
        : id(id), headPosition(position) {}
  };

  using ModuleDeleteFunc = bool (*)(const sndbx::grid::Position &);
  static constexpr std::size_t maxSequencers = 5;

public:
  SequencerManager() = default;

  void addSequencer(std::uint32_t id, sndbx::grid::Position pos) {
    m_Sequencers.emplace_back(id, pos);
  }
  bool deleteSequencer(sndbx::grid::Position pos, ModuleDeleteFunc deleter);

  [[nodiscard]] auto addStep(Sequencer &sequencer, ModuleBuilder &builder)
      -> std::optional<SequencerStepData>;

  [[nodiscard]] bool subtractStep(Sequencer &sequencer,
                                  ModuleDeleteFunc deleter);

  [[nodiscard]] bool isStepAt(sndbx::grid::Position pos) const;
  [[nodiscard]] bool isSequencerAt(sndbx::grid::Position pos) const;

private:
  nst::inplace_vector<SequencerData, maxSequencers> m_Sequencers;
};

#endif