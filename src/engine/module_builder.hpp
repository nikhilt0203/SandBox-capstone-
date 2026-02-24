#ifndef module_builder_hpp_
#define module_builder_hpp_

#include "grid.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

class ModuleBuilder {
private:
  struct ModuleEntry {
    std::uint32_t id;
    sndbx::grid::Position position;

    std::unique_ptr<Module> module;
    Displayable *displayable;
    Controllable *controllable;
    Pressable *pressable;
    Animatable *animatable;
  };

public:
  ModuleBuilder() = default;

  template <typename T, typename... Args>
  T *make(sndbx::grid::Position pos, Args &&...args) {
    if (positionOccupied(pos)) {
      return nullptr;
    }
    auto entry = makeEntry<T>(makeID(), pos, std::forward<Args>(args)...);
    auto module = static_cast<T *>(entry.module.get());
    m_ModuleRegistry.push_back(std::move(entry));
    return module;
  }

  template <typename T> [[nodiscard]] T get(std::uint32_t id) const {
    static_assert(std::is_pointer_v<T>);
    auto it = findWithID(id);
    if (it == m_ModuleRegistry.end()) {
      return nullptr;
    };
    return getFromEntry<T>(*it);
  }

  template <typename T> [[nodiscard]] T get(sndbx::grid::Position pos) const {
    static_assert(std::is_pointer_v<T>);
    auto it = findWithPosition(pos);
    if (it == m_ModuleRegistry.end()) {
      return nullptr;
    };
    return getFromEntry<T>(*it);
  }

  [[nodiscard]] std::optional<std::uint32_t>
  getID(sndbx::grid::Position pos) const {
    std::optional<std::uint32_t> id{};
    auto it = findWithPosition(pos);
    if (it != m_ModuleRegistry.end()) {
      id = it->id;
    };
    return id;
  }

  [[nodiscard]] std::optional<sndbx::grid::Position>
  getPosition(std::uint32_t id) const {
    std::optional<sndbx::grid::Position> position{};
    auto it = findWithID(id);
    if (it != m_ModuleRegistry.end()) {
      position = it->position;
    };
    return position;
  }

  [[nodiscard]] std::optional<sndbx::grid::Position>
  getPosition(Module *m) const {
    auto it = std::find_if(
        m_ModuleRegistry.begin(), m_ModuleRegistry.end(),
        [m](const auto &entry) { return entry.module.get() == m; });

    if (it == m_ModuleRegistry.end()) {
      return std::nullopt;
    }
    return it->position;
  }

  [[nodiscard]] bool destroy(sndbx::grid::Position pos) {
    auto it = findWithPosition(pos);
    if (it == m_ModuleRegistry.end()) {
      return false;
    }
    m_ModuleRegistry.erase(it);
    return true;
  }

  [[nodiscard]] bool positionOccupied(sndbx::grid::Position pos) const {
    return findWithPosition(pos) != m_ModuleRegistry.end();
  }

private:
  template <typename T, typename... Args>
  [[nodiscard]] ModuleEntry
  makeEntry(std::uint32_t id, sndbx::grid::Position pos, Args &&...args) const {
    static_assert(std::is_base_of_v<Module, T>,
                  "Entry must be derived from Module.");

    std::unique_ptr<T> module =
        std::make_unique<T>(std::forward<Args>(args)...);
    module->setID(id);

    Displayable *displayable = nullptr;
    Controllable *controllable = nullptr;
    Pressable *pressable = nullptr;
    Animatable *animatable = nullptr;

    if constexpr (std::is_base_of_v<Displayable, T>) {
      displayable = module.get();
    }
    if constexpr (std::is_base_of_v<Controllable, T>) {
      controllable = module.get();
    }
    if constexpr (std::is_base_of_v<Pressable, T>) {
      pressable = module.get();
    }
    if constexpr (std::is_base_of_v<Animatable, T>) {
      animatable = module.get();
    }

    return ModuleEntry{
      id,           
      pos,       
      std::move(module), 
      displayable,       
      controllable, 
      pressable, 
      animatable
    };
  }

  template <typename T>
  [[nodiscard]] T getFromEntry(const ModuleEntry &entry) const {
    static_assert(
        std::is_same_v<T, Module *> || std::is_same_v<T, Displayable *> ||
            std::is_same_v<T, Controllable *> ||
            std::is_same_v<T, Pressable *> || std::is_same_v<T, Animatable *>,
        "Type parameter is invalid.");

    if constexpr (std::is_same_v<T, Module *>) {
      return entry.module.get();
    }
    if constexpr (std::is_same_v<T, Displayable *>) {
      return entry.displayable;
    }
    if constexpr (std::is_same_v<T, Controllable *>) {
      return entry.controllable;
    }
    if constexpr (std::is_same_v<T, Pressable *>) {
      return entry.pressable;
    }
    if constexpr (std::is_same_v<T, Animatable *>) {
      return entry.animatable;
    }
  }

  [[nodiscard]] std::vector<ModuleEntry>::const_iterator
  findWithID(std::uint32_t id) const {
    return std::find_if(
        m_ModuleRegistry.begin(), m_ModuleRegistry.end(),
        [&id](const ModuleEntry &entry) { return entry.id == id; });
  }

  [[nodiscard]] std::vector<ModuleEntry>::const_iterator
  findWithPosition(sndbx::grid::Position pos) const {
    return std::find_if(
        m_ModuleRegistry.begin(), m_ModuleRegistry.end(),
        [&pos](const ModuleEntry &entry) { return entry.position == pos; });
  }

  [[nodiscard]] std::uint32_t makeID() const {
    static std::uint32_t lastID = 0;
    return lastID++;
  }

public:
  std::vector<ModuleEntry> m_ModuleRegistry{};
};

#endif