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
    std::uint32_t id;
    sndbx::grid::Position headPosition;
    sndbx::vector_32U<KeyboardKeyData> keys{};

    KeyboardData() : id{}, headPosition{} {}
    KeyboardData(std::uint32_t id, sndbx::grid::Position position)
    : id(id), headPosition(position) {}
  };

  using ModuleDeleteFunc = bool(*)(sndbx::grid::Position);

public:
  KeyboardManager() = default;

  void addKeyboard(std::uint32_t id, sndbx::grid::Position pos) { m_Keyboards.emplace_back(id, pos); }
  bool deleteKeyboard(sndbx::grid::Position pos, ModuleDeleteFunc deleter);

  [[nodiscard]] std::optional<KeyboardKeyData> addKey(Keyboard& keyboard, ModuleBuilder& builder);
  [[nodiscard]] bool subtractKey(Keyboard& keyboard, ModuleDeleteFunc deleter);

  void changeScale(Keyboard::Scale scale, std::uint32_t keyboardID);

  [[nodiscard]] bool isKeyAt(sndbx::grid::Position pos) const noexcept;
  [[nodiscard]] bool isKeyboardAt(sndbx::grid::Position pos) const noexcept;

private:
  [[nodiscard]] float nextKeyAmplitude(Keyboard::Scale scale, const KeyboardData* keyboard) const;
  [[nodiscard]] KeyboardData* getKeyboardData(std::uint32_t id);

private:
  static constexpr std::size_t maxKeyboards = 5;
  sndbx::fixed_vector<KeyboardData, maxKeyboards> m_Keyboards;
};


#endif