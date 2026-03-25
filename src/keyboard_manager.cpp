#include "keyboard_manager.hpp"

constexpr float SEMITONE = 1.0f / 100.0f;

using Intervals = std::array<std::uint8_t, 7>;

struct ScaleIntervalPattern
{
  Keyboard::Scale scale;
  std::uint8_t length;
  Intervals intervals;
};

constexpr ScaleIntervalPattern major{ Keyboard::Scale::Major, 7, Intervals{2, 2, 1, 2, 2, 2, 1} };
constexpr ScaleIntervalPattern minor{ Keyboard::Scale::Minor, 7, Intervals{2, 1, 2, 2, 1, 2, 2} };
constexpr ScaleIntervalPattern majorPentatonic{ Keyboard::Scale::Major, 5, Intervals{2, 2, 3, 2, 3, 0, 0} };
constexpr ScaleIntervalPattern minorPentatonic{ Keyboard::Scale::Major, 5, Intervals{3, 2, 2, 3, 2, 0, 0} };

constexpr std::array<ScaleIntervalPattern, 4> scaleIntervalPatterns{ major, minor, majorPentatonic, minorPentatonic };

[[nodiscard]] const ScaleIntervalPattern& scaleIntervalPattern(Keyboard::Scale scale)
{
  for (const auto& intervals : scaleIntervalPatterns)
  {
    if (intervals.scale == scale) { return intervals; }
  }
  return scaleIntervalPatterns[0];
}

[[nodiscard]] float nextKeyAmplitude(Keyboard::Scale scale, const KeyboardManager::KeyboardData* keyboard)
{
  const auto& scaleData = scaleIntervalPattern(scale);

  const auto& keys = keyboard->keys;
  if (keys.is_empty()) { return 0.0f; }

  const auto lastKey = keys.back().key;
  const auto noteIndex = (keys.size() - 1) % scaleData.length;

  return lastKey->amplitude() + (SEMITONE * scaleData.intervals[noteIndex]);
}

[[nodiscard]] auto getKeyboardData(
    sndbx::fixed_vector<KeyboardManager::KeyboardData, KeyboardManager::maxKeyboards>& keyboards, 
    std::uint32_t id)
 -> KeyboardManager::KeyboardData* 
{
  for (auto& data : keyboards) 
  { 
    if (data.id == id) { return &data; }
  }
  return nullptr;
}

auto KeyboardManager::addKey(Keyboard& keyboard, ModuleBuilder& builder) -> std::optional<KeyboardKeyData> 
{
  auto keyboardData = getKeyboardData(m_Keyboards, keyboard.id());
  if (!keyboardData) { return std::nullopt; }

  auto& keys = keyboardData->keys;

  const auto lastKeyPosition = keys.is_empty() 
                              ? keyboardData->headPosition 
                              : keys.back().position;

  const auto newKeyPosition = sndbx::grid::toPosition(lastKeyPosition.index() + 1);
  if (!sndbx::grid::isBuildableArea(newKeyPosition)) { return std::nullopt; }

  const auto result = builder.make<KeyboardKey>(newKeyPosition);
  if (!result) { return std::nullopt; }
  auto newKey = result.value;

  const auto amplitude = nextKeyAmplitude(keyboard.scale(), keyboardData);
  newKey->setParent(&keyboard);
  newKey->setAmplitude(amplitude);

  const KeyboardKeyData newKeyData{newKey, newKeyPosition};
  keys.push_back(newKeyData);
  return newKeyData;
}

bool KeyboardManager::subtractKey(Keyboard& keyboard, ModuleDeleteFunc deleter)
{
  if (keyboard.numKeys() == 0) { return false; }

  auto keyboardData = getKeyboardData(m_Keyboards, keyboard.id());
  if (!keyboardData) { return false; }

  auto& keys = keyboardData->keys;
  const auto lastKeyPosition = keys.back().position;

  if (!deleter(lastKeyPosition)) { return false; }

  keys.pop_back();
  return true;
}

void KeyboardManager::changeScale(Keyboard::Scale scale, std::uint32_t keyboardID)
{
  auto keyboardData = getKeyboardData(m_Keyboards, keyboardID);
  if (!keyboardData) { return; }

  auto& keys = keyboardData->keys;

  const auto& scaleData = scaleIntervalPattern(scale);

  for (std::size_t i{1}; i < keys.size(); ++i)
  {
    const auto previousIndex = i - 1;

    auto& previousKey = keys.at(previousIndex).key;
    auto& currentKey = keys[i].key;

    const auto noteIndex = previousIndex % scaleData.length;
    const auto interval = SEMITONE * scaleData.intervals[noteIndex];
    const auto amplitude = previousKey->amplitude() + interval;

    currentKey->setAmplitude(amplitude);
  }
}

bool KeyboardManager::isKeyAt(sndbx::grid::Position pos) const noexcept
{
  for (const auto& keyboard : m_Keyboards)
  {
    for (const auto& key : keyboard.keys)
    {
      if (key.position == pos) { return true; }
    }
  }
  return false;
}

bool KeyboardManager::isKeyboardAt(sndbx::grid::Position pos) const noexcept
{
  for (const auto& keyboard : m_Keyboards)
  {
    if (keyboard.headPosition == pos) { return true; }
  }
  return false;
}

bool KeyboardManager::deleteKeyboard(sndbx::grid::Position pos, ModuleDeleteFunc deleter)
{
  auto& keyboards = m_Keyboards;
  auto it = std::find_if(
      keyboards.begin(),
      keyboards.end(),
      [pos](const auto& keyboard){ return keyboard.headPosition == pos; }
    );
  
  if (it == keyboards.end()) { return false; }

  auto& keyboard = *it;
  
  for (const auto& key : keyboard.keys) { deleter(key.position); }

  deleter(keyboard.headPosition);
  keyboards.erase(it);
  return true;
}