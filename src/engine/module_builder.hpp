#ifndef SANDBOX_MODULE_BUILDER_HPP_
#define SANDBOX_MODULE_BUILDER_HPP_

#include "grid.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include "engine/module_types.hpp"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

class ModuleBuilder 
{
public:
  struct ModuleEntry 
  {
    std::uint32_t id;
    std::size_t typeIndex;
    sndbx::grid::Position position;

    Module* module;
    Displayable* displayable;
    Controllable* controllable;
    Pressable* pressable;
    Animatable* animatable;
    Serializable* serializable;
  };

  static constexpr std::size_t maxModules = 64;
  using ModuleRegistry = std::array<std::optional<ModuleEntry>, maxModules>;

public:
  ModuleBuilder() = default;

  template <typename T, typename... Args>
  T* make(sndbx::grid::Position pos, Args &&...args) 
  {
    if (positionOccupied(pos)) { return nullptr; }

    auto newModule = m_ModulePools.acquire<T>();
    if (!newModule) { return nullptr; }

    auto entry = makeEntry<T>(newModule, pos, std::forward<Args>(args)...);
    auto module = static_cast<T*>(entry.module);

    for (auto& slot : m_ModuleRegistry) 
    {
      if (slot.has_value()) { continue; } 
      slot = entry; 
      return module; 
    }
    return nullptr;
  }

  template <typename T> 
  [[nodiscard]] T get(std::uint32_t id) const 
  {
    static_assert(std::is_pointer_v<T>);
    auto it = findWithID(id);
    if (it == m_ModuleRegistry.end()) { return nullptr; };
    return getFromEntry<T>(it->value());
  }

  template <typename T> 
  [[nodiscard]] T get(sndbx::grid::Position pos) const 
  {
    static_assert(std::is_pointer_v<T>);
    auto it = findWithPosition(pos);
    if (it == m_ModuleRegistry.end()) { return nullptr; }
    return getFromEntry<T>(it->value());
  }

  [[nodiscard]] const ModuleEntry* getEntry(sndbx::grid::Position pos) const 
  {
    auto it = findWithPosition(pos);
    if (it != m_ModuleRegistry.end()) { return &it->value(); }
    return nullptr;
  }

  [[nodiscard]] const ModuleEntry* getEntry(std::uint32_t id) const 
  {
    auto it = findWithID(id);
    if (it != m_ModuleRegistry.end()) { return &it->value(); }
    return nullptr;
  }

  template <typename T> 
  [[nodiscard]] T getFromEntry(const ModuleEntry& entry) const 
  {
    static_assert(
      std::is_same_v<T, Module*> || 
      std::is_same_v<T, Displayable*> ||
      std::is_same_v<T, Controllable*> ||
      std::is_same_v<T, Pressable*> || 
      std::is_same_v<T, Animatable*> || 
      std::is_same_v<T, Serializable*>,
      "Type parameter is invalid.");

    if constexpr (std::is_same_v<T, Module*>) { return entry.module; }
    if constexpr (std::is_same_v<T, Displayable*>) { return entry.displayable; }
    if constexpr (std::is_same_v<T, Controllable*>) { return entry.controllable; }
    if constexpr (std::is_same_v<T, Pressable*>) { return entry.pressable; }
    if constexpr (std::is_same_v<T, Animatable*>) { return entry.animatable; }
    if constexpr (std::is_same_v<T, Serializable*>) { return entry.serializable; }
  }

  [[nodiscard]] bool destroy(sndbx::grid::Position pos) 
  {
    using namespace sndbx::engine;

    auto it = std::find_if(
        m_ModuleRegistry.begin(), 
        m_ModuleRegistry.end(),
        [&pos](const auto& entry){
          if (!entry) return false;
          return entry->position == pos; 
        }
      );

    if (it == m_ModuleRegistry.end()) { return false; }

    auto& entry = *it;
    const auto module = entry->module;

    switch (entry->typeIndex)
    {
      case typeIndexOf<Oscillator>():   m_ModulePools.release(static_cast<Oscillator*>(module)); break;
      case typeIndexOf<LFO>():          m_ModulePools.release(static_cast<LFO*>(module)); break;
      case typeIndexOf<Mixer>():        m_ModulePools.release(static_cast<Mixer*>(module)); break;
      case typeIndexOf<Keyboard>():     m_ModulePools.release(static_cast<Keyboard*>(module)); break;
      case typeIndexOf<KeyboardKey>():  m_ModulePools.release(static_cast<KeyboardKey*>(module)); break;
      case typeIndexOf<Oscilloscope>(): m_ModulePools.release(static_cast<Oscilloscope*>(module)); break;
      case typeIndexOf<VCF>():          m_ModulePools.release(static_cast<VCF*>(module)); break;
      case typeIndexOf<Envelope>():     m_ModulePools.release(static_cast<Envelope*>(module)); break;
      case typeIndexOf<Mult>():         m_ModulePools.release(static_cast<Mult*>(module)); break;
      case typeIndexOf<USBOut>():       m_ModulePools.release(static_cast<USBOut*>(module)); break;
    }

    entry.reset();

    return true;
  }

  [[nodiscard]] bool positionOccupied(sndbx::grid::Position pos) const 
  {
    return findWithPosition(pos) != m_ModuleRegistry.end();
  }

  [[nodiscard]] const ModuleRegistry& registry() const { return m_ModuleRegistry; }

private:
  template <typename T, typename... Args>
  [[nodiscard]] ModuleEntry makeEntry(T* module, sndbx::grid::Position pos) const
  {
    static_assert(std::is_base_of_v<Module, T>, "Entry must be derived from Module.");

    const auto id = makeID();
    module->setID(id);

    Displayable* displayable{};
    Controllable* controllable{};
    Pressable* pressable{};
    Animatable* animatable{};
    Serializable* serializable{};

    if constexpr (std::is_base_of_v<Displayable, T>) { displayable = module; }
    if constexpr (std::is_base_of_v<Controllable, T>) { controllable = module; }
    if constexpr (std::is_base_of_v<Pressable, T>) { pressable = module; }
    if constexpr (std::is_base_of_v<Animatable, T>) { animatable = module; }
    if constexpr (std::is_base_of_v<Serializable, T>) { serializable = module; }

    return ModuleEntry{
      id,
      sndbx::engine::typeIndexOf<T>(),
      pos,       
      module, 
      displayable,       
      controllable, 
      pressable, 
      animatable,
      serializable
    };
  }

  [[nodiscard]] ModuleRegistry::const_iterator findWithID(std::uint32_t id) const 
  {
    return std::find_if(
        m_ModuleRegistry.begin(), 
        m_ModuleRegistry.end(),
        [&id](const auto& entry){
          if (!entry) return false;
          return entry->id == id; 
        }
      );
  }

  [[nodiscard]] ModuleRegistry::const_iterator findWithPosition(sndbx::grid::Position pos) const 
  {
    return std::find_if(
        m_ModuleRegistry.begin(), 
        m_ModuleRegistry.end(),
        [&pos](const auto& entry){
          if (!entry) return false;
          return entry->position == pos; 
        }
      );
  }

  [[nodiscard]] std::uint32_t makeID() const 
  {
    static std::uint32_t lastID = 0;
    return lastID++;
  }

private:
  inline static ModuleRegistry m_ModuleRegistry{};
  inline static sndbx::engine::ModulePools m_ModulePools;
};

namespace engine
{
 
}

#endif