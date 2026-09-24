#include "sequencer_manager.hpp"

[[nodiscard]] auto getSequencerData(
    nst::inplace_vector<SequencerManager::SequencerData,
                        SequencerManager::maxSequencers> &sequencers,
    std::uint32_t id) -> SequencerManager::SequencerData * {
  for (auto &data : sequencers) {
    if (data.id == id) {
      return &data;
    }
  }
  return nullptr;
}

auto SequencerManager::addStep(Sequencer &sequencer, ModuleBuilder &builder)
    -> std::optional<SequencerStepData> {
  auto sequencerData = getSequencerData(m_Sequencers, sequencer.id());
  if (!sequencerData) {
    return std::nullopt;
  }

  auto &steps = sequencerData->steps;

  const auto lastStepPosition =
      steps.is_empty() ? sequencerData->headPosition : steps.back().position;

  const auto newStepPosition =
      sndbx::grid::toPosition(lastStepPosition.index() + 1);
  if (!sndbx::grid::isBuildableArea(newStepPosition)) {
    return std::nullopt;
  }

  const auto result = builder.make<SequencerStep>(newStepPosition);
  if (!result) {
    return std::nullopt;
  }
  auto newStep = *result;
  sequencer.registerStep(newStep);

  const SequencerStepData newStepData{newStep, newStepPosition};
  steps.push_back(newStepData);
  return newStepData;
}

bool SequencerManager::subtractStep(Sequencer &sequencer,
                                    ModuleDeleteFunc deleter) {
  if (sequencer.numSteps() == 0) {
    return false;
  }

  auto sequencerData = getSequencerData(m_Sequencers, sequencer.id());
  if (!sequencerData) {
    return false;
  }

  auto &steps = sequencerData->steps;
  const auto lastStep = steps.back();
  lastStep.step->off();

  if (!deleter(lastStep.position)) {
    return false;
  }

  steps.pop_back();
  return true;
}

bool SequencerManager::isStepAt(sndbx::grid::Position pos) const {
  for (const auto &sequencer : m_Sequencers) {
    for (const auto &step : sequencer.steps) {
      if (step.position == pos) {
        return true;
      }
    }
  }
  return false;
}

bool SequencerManager::isSequencerAt(sndbx::grid::Position pos) const {
  for (const auto &sequencer : m_Sequencers) {
    if (sequencer.headPosition == pos) {
      return true;
    }
  }
  return false;
}

bool SequencerManager::deleteSequencer(sndbx::grid::Position pos,
                                       ModuleDeleteFunc deleter) {
  auto &sequencers = m_Sequencers;
  auto it = std::find_if(
      sequencers.begin(), sequencers.end(),
      [pos](const auto &sequencer) { return sequencer.headPosition == pos; });

  if (it == sequencers.end()) {
    return false;
  }

  auto &sequencer = *it;

  for (const auto &step : sequencer.steps) {
    deleter(step.position);
  }

  deleter(sequencer.headPosition);
  sequencers.erase(it);
  return true;
}