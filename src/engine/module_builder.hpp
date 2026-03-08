#ifndef SANDBOX_MODULE_BUILDER_HPP_
#define SANDBOX_MODULE_BUILDER_HPP_

#include "grid.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include "engine/module_types.hpp"
#include "core/error_result.hpp"

#include <array>
#include <type_traits>

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

  static constexpr std::size_t maxModules = 56;
  using ModuleRegistry = sndbx::fixed_vector<ModuleEntry, maxModules>;
  using ModuleReleaseTable = std::array<void(*)(Module*), sndbx::engine::numModules()>;

public:
  ModuleBuilder() = default;

  /**
   * @brief Creates a module at the specified position.
   *      
   * Pulls a module of type T from the preallocated pool and creates a registry entry.
   * 
   * @tparam T  Module type
   * @param pos Position to create at
   * @param args Forwarded constructor args
   * 
   * @return sndbx::Result<T*> 
   * 
   * @retval T* pointer to the new module
   * @retval sndbx::Error::BUILDER_INVALID_POS 
   * @retval sndbx::Error::BUILDER_POOL_EXHAUSTED 
   * @retval sndbx::Error::BUILDER_REGISTRY_FULL 
   */
  template <typename T, typename... Args>
  [[nodiscard]] sndbx::Result<T*> make(sndbx::grid::Position pos, Args &&...args) 
  {
    if (find(pos) != m_ModuleRegistry.end()) { return {sndbx::Error::BUILDER_INVALID_POS}; }

    const auto newModule = m_ModulePools.acquire<T>();
    if (!newModule) { return {sndbx::Error::BUILDER_POOL_EXHAUSTED}; }

    const auto entry = makeEntry<T>(newModule, pos, std::forward<Args>(args)...);
    const auto module = static_cast<T*>(entry.module);

    if (m_ModuleRegistry.push_back(entry)) { return { module }; }
    else
    {
      m_ModulePools.release(module);
      return {sndbx::Error::BUILDER_REGISTRY_FULL};
    }
  }

  /**
   * @brief Deletes the module at the given position.
   * 
   * @param pos Position of module to delete
   * @return true (deletion was successful)
   * @return false 
   */
  [[nodiscard]] bool destroy(sndbx::grid::Position pos);

  /**
   * @brief 
   * 
   * @tparam T 
   * @param id 
   * @return T 
   */
  template <typename T> 
  [[nodiscard]] T get(std::uint32_t id) const 
  {
    static_assert(std::is_pointer_v<T>);
    auto it = find(id);
    if (it == m_ModuleRegistry.end()) { return nullptr; }
    return getFromEntry<T>(*it);
  }

  template <typename T> 
  [[nodiscard]] T get(sndbx::grid::Position pos) const 
  {
    static_assert(std::is_pointer_v<T>);
    auto it = find(pos);
    if (it == m_ModuleRegistry.end()) { return nullptr; }
    return getFromEntry<T>(*it);
  }

  [[nodiscard]] const ModuleEntry* getEntry(sndbx::grid::Position pos) const;
  [[nodiscard]] const ModuleEntry* getEntry(std::uint32_t id) const;

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

    if constexpr (std::is_same_v<T, Module*>)            { return entry.module; }
    else if constexpr (std::is_same_v<T, Displayable*>)  { return entry.displayable; }
    else if constexpr (std::is_same_v<T, Controllable*>) { return entry.controllable; }
    else if constexpr (std::is_same_v<T, Pressable*>)    { return entry.pressable; }
    else if constexpr (std::is_same_v<T, Animatable*>)   { return entry.animatable; }
    else if constexpr (std::is_same_v<T, Serializable*>) { return entry.serializable; }
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

    if constexpr (std::is_base_of_v<Displayable, T>)  { displayable = module; }
    if constexpr (std::is_base_of_v<Controllable, T>) { controllable = module; }
    if constexpr (std::is_base_of_v<Pressable, T>)    { pressable = module; }
    if constexpr (std::is_base_of_v<Animatable, T>)   { animatable = module; }
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

  [[nodiscard]] ModuleRegistry::const_iterator find(std::uint32_t id) const;
  [[nodiscard]] ModuleRegistry::const_iterator find(sndbx::grid::Position pos) const;

  [[nodiscard]] std::uint32_t makeID() const;

private:
  template<typename T>
  static void releaseModule(Module* m) { m_ModulePools.release(static_cast<T*>(m)); }

  template <std::size_t... Is>
  static constexpr ModuleReleaseTable createReleaseTable(std::index_sequence<Is...>) 
  {
    return { &releaseModule<sndbx::engine::ModuleTypes::get<Is>>... };
  }

private:
  ModuleRegistry m_ModuleRegistry{};
  static sndbx::engine::ModulePools m_ModulePools;
  static const ModuleReleaseTable m_ModuleReleaseFuncs;
};

inline const ModuleBuilder::ModuleReleaseTable ModuleBuilder::m_ModuleReleaseFuncs 
  = createReleaseTable(std::make_index_sequence<sndbx::engine::numModules()>{});

#endif