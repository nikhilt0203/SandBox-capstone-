#ifndef SANDBOX_MODULE_BUILDER_HPP_
#define SANDBOX_MODULE_BUILDER_HPP_

#include <array>
#include <type_traits>
#include <utility>

#include "engine/module_types.hpp"
#include "grid.hpp"
#include "modules/dep/module.hpp"
#include "modules/dep/module_interfaces.hpp"
#include <nst/expected.hpp>

/**
 * @brief Responsible for creating and destroying modules and maintains
 *        a registry of all active modules.
 *
 * This object does not dynamically allocate. Module creation pulls from
 * preallocated pools and the internal registry is of fixed size.
 */
class ModuleBuilder {
public:
  enum class Error {
    NONE,
    REGISTRY_FULL,
    POOL_EXHAUSTED,
    INVALID_POSITION,
    INVALID_BANK_INDEX
  };
  /**
   * @brief A single entry in the registry. Contains the module
   *        information and pointers to interfaces (each may be null
   *        if the module does not implement them).
   */
  struct ModuleEntry {
    mutable std::uint32_t id;
    sndbx::ModuleType type;
    sndbx::grid::Position position;

    Module *module;
    Displayable *displayable;
    Controllable *controllable;
    Pressable *pressable;
    Animatable *animatable;
    Serializable *serializable;

    void setID(std::uint32_t newID) const {
      id = newID;
      module->setID(newID);
    }
  };

  static constexpr std::size_t maxModules = 56;
  using ModuleRegistry = nst::inplace_vector<ModuleEntry, maxModules>;

public:
  ModuleBuilder() = default;

  /**
   * @brief Creates a module at the specified position and returns a pointer to
   * it.
   *
   * Pulls a module of type T from the preallocated pool and creates a registry
   * entry.
   *
   * @tparam T  Module type.
   * @param pos Position to create at.
   *
   * @return nst::expected<T *, Error> Type containing the result.
   *
   * @retval T* Pointer to the new module.
   * @retval nst::Error::BUILDER_INVALID_POS Position is already occupied.
   * @retval nst::Error::BUILDER_POOL_EXHAUSTED No more modules of the given
   * type can be created.
   * @retval nst::Error::BUILDER_REGISTRY_FULL No more modules can be created.
   */
  template <typename T>
  [[nodiscard]] nst::expected<T *, Error>
  make(const sndbx::grid::Position &pos) {
    return makeImpl<T>(pos, makeModuleID());
  }

  /**
   * @brief Creates a module at the specified position and returns a pointer to
   * it.
   *
   * Pulls a module of type T from the preallocated pool and creates a registry
   * entry.
   *
   * @tparam T  Module type.
   * @param pos Position to create at.
   * @param id  ID to create module with
   * @param args Forwarded constructor args.
   *
   * @return nst::error_or<T*> Type containing the result.
   *
   * @retval T* Pointer to the new module.
   * @retval nst::Error::BUILDER_INVALID_POS Position is already occupied.
   * @retval nst::Error::BUILDER_POOL_EXHAUSTED No more modules of the given
   * type can be created.
   * @retval nst::Error::BUILDER_REGISTRY_FULL No more modules can be created.
   */
  template <typename T, typename... Args>
  [[nodiscard]] nst::expected<T *, Error> make(const sndbx::grid::Position &pos,
                                               std::size_t id) {
    return makeImpl<T>(pos, id);
  }

  /**
   * @brief Deletes the module at the given position.
   *
   * @param pos Position of module to delete.
   *
   * @return deletion was successful
   */
  [[nodiscard]] bool destroy(const sndbx::grid::Position &pos);

  /**
   * @brief Retrieve a T pointer to a module given its integer ID.
   *
   * @tparam T Type to retrieve.
   * @param id The integer ID of the module.
   *
   * @return T* Pointer to the module.
   *
   * @retval nullptr No module found with the given id.
   */
  template <typename T> [[nodiscard]] T *get(std::uint32_t id) const {
    auto it = find(id);
    if (it == m_ModuleRegistry.end()) {
      return nullptr;
    }
    return getFromEntry<T>(*it);
  }

  /**
   * @brief Retrieve a T pointer to a module given its position.
   *
   * @tparam T Type to retrieve.
   * @param pos The position of the module.
   *
   * @return T* Pointer to the module.
   *
   * @retval nullptr No module found at the given position.
   */
  template <typename T>
  [[nodiscard]] T *get(const sndbx::grid::Position &pos) const {
    auto it = find(pos);
    if (it == m_ModuleRegistry.end()) {
      return nullptr;
    }
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
  [[nodiscard]] const ModuleEntry *
  getModuleEntry(const sndbx::grid::Position &pos) const;

  /**
   * @brief Retrieve the module entry containing the given ID.
   *
   * @param id The ID of the module
   * @return const ModuleEntry*
   *
   * @retval nullptr No module found with the given ID.
   */
  [[nodiscard]] const ModuleEntry *getModuleEntry(std::uint32_t id) const;

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
  [[nodiscard]] T *getFromEntry(const ModuleEntry &entry) const {
    static_assert(
        std::is_same_v<T, Module> || std::is_same_v<T, Displayable> ||
            std::is_same_v<T, Controllable> || std::is_same_v<T, Pressable> ||
            std::is_same_v<T, Animatable> || std::is_same_v<T, Serializable>,
        "Type parameter is invalid.");

    if constexpr (std::is_same_v<T, Module>) {
      return entry.module;
    } else if constexpr (std::is_same_v<T, Displayable>) {
      return entry.displayable;
    } else if constexpr (std::is_same_v<T, Controllable>) {
      return entry.controllable;
    } else if constexpr (std::is_same_v<T, Pressable>) {
      return entry.pressable;
    } else if constexpr (std::is_same_v<T, Animatable>) {
      return entry.animatable;
    } else if constexpr (std::is_same_v<T, Serializable>) {
      return entry.serializable;
    }
  }

  [[nodiscard]] const ModuleRegistry &registry() const {
    return m_ModuleRegistry;
  }

private:
  /**
   * @brief Constructs an entry in the module registry, creates an integer ID,
   *        and sets interface pointers based on type T.
   *
   * @tparam T The module type.
   * @param module Pointer to the module.
   * @param pos Position of the module.
   * @param id  ID of the module.
   * @return ModuleEntry
   */
  template <typename T>
  [[nodiscard]] ModuleEntry
  makeEntry(T *module, const sndbx::grid::Position &pos, std::size_t id) const {
    static_assert(std::is_base_of_v<Module, T>,
                  "Entry must be derived from Module.");

    module->setID(id);

    Displayable *displayable{};
    Controllable *controllable{};
    Pressable *pressable{};
    Animatable *animatable{};
    Serializable *serializable{};

    if constexpr (std::is_base_of_v<Displayable, T>) {
      displayable = module;
    }
    if constexpr (std::is_base_of_v<Controllable, T>) {
      controllable = module;
    }
    if constexpr (std::is_base_of_v<Pressable, T>) {
      pressable = module;
    }
    if constexpr (std::is_base_of_v<Animatable, T>) {
      animatable = module;
    }
    if constexpr (std::is_base_of_v<Serializable, T>) {
      serializable = module;
    }

    return ModuleEntry{id,        sndbx::type_id<T>, pos,
                       module,    displayable,       controllable,
                       pressable, animatable,        serializable};
  }

  template <typename T>
  [[nodiscard]] nst::expected<T *, Error>
  makeImpl(const sndbx::grid::Position &pos, std::size_t id) {
    if (m_ModuleRegistry.is_full()) {
      return nst::unexpected{Error::REGISTRY_FULL};
    }
    if (find(pos) != m_ModuleRegistry.end()) {
      return nst::unexpected{Error::INVALID_POSITION};
    }

    const auto newModule = sndbx::engine::pool_acquire<T>();

    if (!newModule) {
      return nst::unexpected{Error::POOL_EXHAUSTED};
    }

    m_ModuleRegistry.emplace_back(makeEntry<T>(newModule, pos, id));

    return newModule;
  }

  /**
   * @brief Returns an iterator pointing to the module entry with the matching
   * ID.
   *
   * @param id The module ID.
   * @return ModuleRegistry::const_iterator
   */
  [[nodiscard]] ModuleRegistry::const_iterator find(std::uint32_t id) const;

  /**
   * @brief Returns an iterator pointing to the module entry with the matching
   * position.
   *
   * @param pos The module position.
   * @return ModuleRegistry::const_iterator
   */
  [[nodiscard]] ModuleRegistry::const_iterator
  find(const sndbx::grid::Position &pos) const;

  [[nodiscard]] std::uint32_t makeModuleID() const;

private:
  ModuleRegistry m_ModuleRegistry;
};
//
#endif
