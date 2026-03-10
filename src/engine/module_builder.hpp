#ifndef SANDBOX_MODULE_BUILDER_HPP_
#define SANDBOX_MODULE_BUILDER_HPP_

#include "grid.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include "engine/module_types.hpp"
#include "core/error_result.hpp"

#include <array>
#include <type_traits>

/**
 * @brief Responsible for creating and destroying modules and maintains
 *        a registry of all active modules.
 * 
 * This object does not dynamically allocate. Module creation pulls from preallocated pools
 * and the internal registry is of fixed size.
 */
class ModuleBuilder 
{
public:
  /**
   * @brief A single entry in the registry. Contains the module 
   *        information and pointers to interfaces (each may be null
   *        if the module does not implement them).
   */
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
  using ModuleReleaseTable = std::array<void(*)(Module*), sndbx::engine::numModuleTypes()>;

public:
  ModuleBuilder() = default;

  /**
   * @brief Creates a module at the specified position and returns a pointer to it.
   *      
   * Pulls a module of type T from the preallocated pool and creates a registry entry.
   * 
   * @tparam T  Module type
   * @param pos Position to create at
   * @param args Forwarded constructor args
   * 
   * @return sndbx::Result<T*> Type containing the result.
   * 
   * @retval T* Pointer to the new module.
   * @retval sndbx::Error::BUILDER_INVALID_POS Position is already occupied.
   * @retval sndbx::Error::BUILDER_POOL_EXHAUSTED No more modules of the given type can be created.
   * @retval sndbx::Error::BUILDER_REGISTRY_FULL No more modules can be created.
   */
  template <typename T, typename... Args>
  [[nodiscard]] sndbx::Result<T*> make(sndbx::grid::Position pos, Args &&...args) 
  {
    if (m_ModuleRegistry.is_full()) { return sndbx::Error::BUILDER_REGISTRY_FULL; }
    if (find(pos) != m_ModuleRegistry.end()) { return sndbx::Error::BUILDER_INVALID_POS; }

    const auto newModule = m_ModulePools.acquire<T>();
    if (!newModule) { return sndbx::Error::BUILDER_POOL_EXHAUSTED; }

    const auto entry = makeEntry<T>(newModule, pos, std::forward<Args>(args)...);
    const auto module = static_cast<T*>(entry.module);

    if (!m_ModuleRegistry.push_back(entry)) { m_ModulePools.release(module); return nullptr;};
    return module;
  }

  /**
   * @brief Deletes the module at the given position.
   * 
   * @param pos Position of module to delete.
   * 
   * @return true (deletion was successful)
   * @return false 
   */
  [[nodiscard]] bool destroy(sndbx::grid::Position pos);

  /**
   * @brief Retrieve a T pointer to a module given its integer ID.
   * 
   * T must be of type Module or a valid Module interface type.
   * 
   * @tparam T Type to retrieve.
   * @param id The integer ID of the module.
   * 
   * @return T* Pointer to the module. 
   * 
   * @retval nullptr No module found with the given id.
   */
  template <typename T> 
  [[nodiscard]] T* get(std::uint32_t id) const 
  {
    auto it = find(id);
    if (it == m_ModuleRegistry.end()) { return nullptr; }
    return getFromEntry<T>(*it);
  }

  /**
   * @brief Retrieve a T pointer to a module given its position.
   * 
   * T must be of type Module or a valid Module interface type.
   * 
   * @tparam T Type to retrieve.
   * @param pos The position of the module.
   * 
   * @return T* Pointer to the module. 
   * 
   * @retval nullptr No module found at the given position.
   */
  template <typename T> 
  [[nodiscard]] T* get(sndbx::grid::Position pos) const 
  {
    auto it = find(pos);
    if (it == m_ModuleRegistry.end()) { return nullptr; }
    return getFromEntry<T>(*it);
  }

  /**
   * @brief Retrieve the module entry containing the given position.
   * 
   * @param pos The position of the module
   * @return const ModuleEntry*
   * 
   * @retval nullptr No module found at the given position.
   */
  [[nodiscard]] const ModuleEntry* getEntry(sndbx::grid::Position pos) const;

  /**
   * @brief Retrieve the module entry containing the given ID.
   * 
   * @param id The ID of the module
   * @return const ModuleEntry*
   * 
   * @retval nullptr No module found with the given ID.
   */
  [[nodiscard]] const ModuleEntry* getEntry(std::uint32_t id) const;


  /**
   * @brief Retrieve a T pointer from the given module entry.
   * 
   * T must be type Module or a valid module interface.
   * 
   * @tparam T The type to retrieve.
   * @param entry The module entry.
   * 
   * @return T* 
   * 
   * @retval nullptr The module does not implement T
   */
  template <typename T> 
  [[nodiscard]] T* getFromEntry(const ModuleEntry& entry) const 
  {
    static_assert(
      std::is_same_v<T, Module> || 
      std::is_same_v<T, Displayable> ||
      std::is_same_v<T, Controllable> ||
      std::is_same_v<T, Pressable> || 
      std::is_same_v<T, Animatable> || 
      std::is_same_v<T, Serializable>,
      "Type parameter is invalid.");

    if constexpr (std::is_same_v<T, Module>)            { return entry.module; }
    else if constexpr (std::is_same_v<T, Displayable>)  { return entry.displayable; }
    else if constexpr (std::is_same_v<T, Controllable>) { return entry.controllable; }
    else if constexpr (std::is_same_v<T, Pressable>)    { return entry.pressable; }
    else if constexpr (std::is_same_v<T, Animatable>)   { return entry.animatable; }
    else if constexpr (std::is_same_v<T, Serializable>) { return entry.serializable; }
  }

  [[nodiscard]] const ModuleRegistry& registry() const { return m_ModuleRegistry; }

private:
  /**
   * @brief Constructs an entry in the module registry, creates an integer ID,
   *        and sets interface pointers based on type T.
   * 
   * @tparam T The module type.
   * @param module Pointer to the module.
   * @param pos Position of the module.
   * @return ModuleEntry 
   */
  template <typename T>
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

  /**
   * @brief Returns an iterator pointing to the module entry with the matching ID.
   * 
   * @param id The module ID.
   * @return ModuleRegistry::const_iterator 
   */
  [[nodiscard]] ModuleRegistry::const_iterator find(std::uint32_t id) const;

    /**
   * @brief Returns an iterator pointing to the module entry with the matching position.
   * 
   * @param pos The module position.
   * @return ModuleRegistry::const_iterator 
   */
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
  ModuleRegistry m_ModuleRegistry;
  static sndbx::engine::ModulePools m_ModulePools;
  static const ModuleReleaseTable m_ModuleReleaseFuncs;
};

inline const ModuleBuilder::ModuleReleaseTable ModuleBuilder::m_ModuleReleaseFuncs 
  = createReleaseTable(std::make_index_sequence<sndbx::engine::numModuleTypes()>{});

#endif