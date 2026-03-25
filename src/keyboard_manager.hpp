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
  static constexpr std::size_t maxKeyboards = 5;

public:
  KeyboardManager() = default;

  void addKeyboard(std::uint32_t id, sndbx::grid::Position pos) { m_Keyboards.emplace_back(id, pos); }
  bool deleteKeyboard(sndbx::grid::Position pos, ModuleDeleteFunc deleter);

  [[nodiscard]] auto addKey(Keyboard& keyboard, ModuleBuilder& builder) -> std::optional<KeyboardKeyData>;
  [[nodiscard]] bool subtractKey(Keyboard& keyboard, ModuleDeleteFunc deleter);

  [[nodiscard]] bool isKeyAt(sndbx::grid::Position pos) const noexcept;
  [[nodiscard]] bool isKeyboardAt(sndbx::grid::Position pos) const noexcept;

  void changeScale(Keyboard::Scale scale, std::uint32_t keyboardID);

private:
  sndbx::fixed_vector<KeyboardData, maxKeyboards> m_Keyboards;
};

#endif