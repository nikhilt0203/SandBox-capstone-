#ifndef SANDBOX_KEYBOARD_MANAGER_HPP_
#define SANDBOX_KEYBOARD_MANAGER_HPP_
#include "modules/keyboard.hpp"
#include "engine/module_builder.hpp"
#include "grid.hpp"


class KeyboardManager
{
public:
  struct KeyboardKeyData 
  {
    KeyboardKey* key;
    sndbx::grid::Position position;
  };

  struct KeyboardData
  {
    sndbx::grid::Position headPosition;
    std::vector<KeyboardKeyData> keys;
  };

  using ModuleDeleteFunc = bool(*)(sndbx::grid::Position);

public:
  KeyboardManager() = default;

  void addKeyboard(std::uint32_t id, sndbx::grid::Position pos) { m_Keyboards[id].headPosition = pos; }

  [[nodiscard]] std::optional<KeyboardKeyData> addKey(Keyboard& keyboard, ModuleBuilder& builder)
  {
    const auto keyboardID = keyboard.id();

    auto it = m_Keyboards.find(keyboardID);
    if (it == m_Keyboards.end()) { return std::nullopt; }

    auto&[_, keyboardData] = *it;
    auto& keys= keyboardData.keys;

    const auto lastKeyPosition = keys.empty() 
                                ? keyboardData.headPosition 
                                : keys.back().position;

    const auto newKeyPosition = sndbx::grid::toPosition(lastKeyPosition.index() + 1);

    if (!sndbx::grid::isBuildableArea(newKeyPosition)) { return std::nullopt; }

    auto newKey = builder.make<KeyboardKey>(newKeyPosition, keyboard);
    if (!newKey) { return std::nullopt; }

    const auto amplitude = nextKeyAmplitude(keyboard.scale(), keyboardData);
    newKey->setAmplitude(amplitude);

    const KeyboardKeyData newKeyData{newKey, newKeyPosition};
    keys.push_back(newKeyData);

    return newKeyData;
  }

  [[nodiscard]] bool subtractKey(Keyboard& keyboard, ModuleDeleteFunc deleter)
  {
    if (keyboard.numKeys() == 0) { return false; }

    const auto keyboardID = keyboard.id();

    auto it = m_Keyboards.find(keyboardID);
    if (it == m_Keyboards.end()) { return false; }

    const auto&[_, keyboardData] = *it;
    const auto lastKeyPosition = keyboardData.keys.back().position;

    if (!deleter(lastKeyPosition)) { return false; }

    m_Keyboards[keyboardID].keys.pop_back();
    return true;
  }

  void changeScale(Keyboard::Scale scale, std::uint32_t keyboardID)
  {
    auto it = m_Keyboards.find(keyboardID);
    if (it == m_Keyboards.end()) { return; }
    auto& keys = it->second.keys;

    const auto& scaleIntervals = m_ScaleIntervalPatterns.at(scale);
    const auto scaleLength = scaleIntervals.size();

    for (std::size_t i{1}; i < keys.size(); i++)
    {
      auto& previousKey = keys.at(i - 1).key;
      auto& currentKey = keys[i].key;

      const auto noteIndex = (i - 1) % scaleLength;
      const float amplitude = 
        previousKey->amplitude() + (KeyboardManager::semitone * scaleIntervals[noteIndex]);

      currentKey->setAmplitude(amplitude);
    }
  }

  [[nodiscard]] bool isKeyAt(sndbx::grid::Position pos) const noexcept
  {
    for (const auto& [_, keyboard] : m_Keyboards)
    {
      for (const auto& key : keyboard.keys)
      {
        if (key.position == pos) { return true; }
      }
    }
    return false;
  }

  [[nodiscard]] bool isKeyboardAt(sndbx::grid::Position pos) const noexcept
  {
    for (const auto& [_, keyboard] : m_Keyboards)
    {
      if (keyboard.headPosition == pos) { return true; }
    }
    return false;
  }

  bool deleteKeyboard(sndbx::grid::Position pos, ModuleDeleteFunc deleter)
  {
    auto it = std::find_if(
        m_Keyboards.begin(),
        m_Keyboards.end(),
        [pos](const auto& pair){ return pair.second.headPosition == pos; }
      );
    
    if (it == m_Keyboards.end()) { return false; }

    auto&[_, keyboard] = *it;

    deleter(keyboard.headPosition);
    for (const auto& key : keyboard.keys) { deleter(key.position); }

    m_Keyboards.erase(it);
    return true;
  }

private:
  [[nodiscard]] float nextKeyAmplitude(Keyboard::Scale scale, const KeyboardData& keyboard) const
  {
    const auto& scaleIntervals = m_ScaleIntervalPatterns.at(scale);

    const auto& keys = keyboard.keys;
    if (keys.empty()) { return 0.0f; }

    const auto lastKey = keys.back().key;
    const auto noteIndex = (keys.size() - 1) % scaleIntervals.size();
    return lastKey->amplitude() + (KeyboardManager::semitone * scaleIntervals[noteIndex]);
  }

private:
  using KeyboardID = std::uint32_t;
  using ScaleMap = std::map<Keyboard::Scale, std::vector<std::uint8_t>>;

  std::map<KeyboardID, KeyboardData> m_Keyboards{};

  inline static const ScaleMap m_ScaleIntervalPatterns{
    { Keyboard::Scale::Major,           {2, 2, 1, 2, 2, 2, 1} },
    { Keyboard::Scale::Minor,           {2, 1, 2, 2, 1, 2, 2} },
    { Keyboard::Scale::MajorPentatonic, {2, 2, 3, 2, 3} },
    { Keyboard::Scale::MinorPentatonic, {3, 2, 2, 3, 2} }
  };

  static constexpr float semitone = 1.0f / 100.0f;
};


#endif