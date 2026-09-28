#ifndef SANDBOX_MODULE_REGISTRY_HPP_
#define SANDBOX_MODULE_REGISTRY_HPP_

#include <algorithm>
#include <cstddef>
#include <optional>

#include "nst/inplace_vector.hpp"
#include "nst/poly_view.hpp"
#include "nst/strong_alias.hpp"

#include "module_types.hpp"

namespace sndbx {

struct ModuleID : public nst::strong_alias<std::uint32_t, ModuleID> {
  using strong_alias::strong_alias;
  constexpr operator bool() { return value != 0; }
  bool operator==(const ModuleID &rhs) const { return value == rhs.value; }
  bool operator!=(const ModuleID &rhs) const { return value != rhs.value; }
};

template <std::size_t Capacity, class... ModuleBases> class ModuleRegistry {
public:
  using value_type = nst::poly_view<ModuleBases...>;

  template <class Module>
  [[nodiscard]] auto add(Module *m) -> std::optional<ModuleID> {
    if (modules_.is_full()) {
      return std::nullopt;
    }
    const ModuleID new_id{reinterpret_cast<std::uint32_t>(m)};
    ids_.push_back(new_id);
    modules_.emplace_back(m);
    return new_id;
  }

  [[nodiscard]] const auto &operator[](ModuleID id) const {
    return modules_[index_of(id)];
  }
  [[nodiscard]] auto &operator[](ModuleID id) { return modules_[index_of(id)]; }

  bool erase(ModuleID id) {
    const auto index = index_of(id);
    if (index == ids_.size()) {
      return false;
    }
    modules_.erase(modules_.begin() + index);
    ids_.erase(ids_.begin() + index);
    return true;
  }

  [[nodiscard]] auto find(ModuleID id) const {
    const auto index = index_of(id);
    return (index == ids_.size()) ? cend() : index + cbegin();
  }

  [[nodiscard]] auto find(ModuleID id) {
    const auto index = index_of(id);
    return (index == ids_.size()) ? end() : index + begin();
  }

  void clear() {
    modules_.clear();
    ids_.clear();
  }

  [[nodiscard]] auto begin() { return modules_.begin(); }
  [[nodiscard]] auto end() { return modules_.end(); }
  [[nodiscard]] auto begin() const { return modules_.cbegin(); }
  [[nodiscard]] auto end() const { return modules_.cend(); }
  [[nodiscard]] auto cbegin() const { return begin(); }
  [[nodiscard]] auto cend() const { return end(); }

  [[nodiscard]] const auto &ids() const { return ids_; }

  [[nodiscard]] auto size() const { return modules_.size(); }

private:
  std::size_t index_of(ModuleID id) const {
    const auto it = std::find(ids_.begin(), ids_.end(), id);
    return (it == ids_.end()) ? ids_.size() : std::distance(ids_.begin(), it);
  }

  nst::inplace_vector<value_type, Capacity> modules_;
  nst::inplace_vector<ModuleID, Capacity> ids_;
};

template <std::size_t N, class... ModuleBases> class MappedModuleRegistry {
  using Registry = ModuleRegistry<N, ModuleBases...>;

public:
  constexpr static auto size = N;
  using ModuleView = typename Registry::value_type;

  template <class Module> struct Receipt {
    ModuleID id;
    Module &module;
  };

  enum class Error {
    LOCATION_OCCUPIED,
    REGISTRY_FAILED,
    POOL_EXHAUSTED
  };

  // Creates a Module at the given position. Returns the new id and a reference
  // to the module if successful, Error if not
  template <class Module, typename... Args>
  auto make(std::size_t pos, Args &&...args)
      -> nst::expected<Receipt<Module>, Error> {
    assert(pos < map_.size());

    if (auto &entry = map_[pos]; entry.has_value()) {
      return Error::LOCATION_OCCUPIED;
    } else if (auto *mod =
                   engine::acquire_module<Module>(std::forward<Args>(args)...);
               !mod) {
      return Error::POOL_EXHAUSTED;
    } else if (auto new_id = registry_.add(mod); !new_id) {
      return Error::REGISTRY_FAILED;
    } else {
      entry.emplace(MapEntry{*new_id, module_type<Module>, mod});
      return Receipt<Module>{*new_id, *mod};
    }
  }

  bool erase(ModuleID id) {
    auto it = std::find_if(map_.begin(), map_.end(), [id](const auto &e) {
      return e.has_value() && e->id == id;
    });
    if (it == map_.end()) {
      return false;
    }
    auto &entry = *it;
    engine::release_module(entry->type, entry->ptr);
    entry.reset();
    registry_.erase(id);
    return true;
  }

  // Returns nullptr if not found.
  [[nodiscard]] auto get_module(ModuleID id) const -> const ModuleView * {
    auto it = registry_.find(id);
    if (it == registry_.cend()) {
      return nullptr;
    }
    return it;
  }

  // Returns nullptr if not found.
  [[nodiscard]] auto get_module(ModuleID id) -> ModuleView * {
    auto it = registry_.find(id);
    if (it == registry_.end()) {
      return nullptr;
    }
    return it;
  }

  [[nodiscard]] auto get_id(std::size_t pos) const -> std::optional<ModuleID> {
    const auto &entry = map_[pos];
    if (!entry.has_value()) {
      return std::nullopt;
    }
    return entry->id;
  }

  [[nodiscard]] const auto &operator[](ModuleID id) const {
    return registry_[id];
  }

  [[nodiscard]] auto &operator[](ModuleID id) { return registry_[id]; }

  [[nodiscard]] ModuleID operator[](std::size_t pos) const {
    const auto &entry = map_[pos];
    assert(entry.has_value());
    return map_[pos]->id;
  }

  [[nodiscard]] ModuleType type_at(std::size_t pos) const {
    const auto &entry = map_[pos];
    assert(entry.has_value());
    return entry->type;
  }

  [[nodiscard]] static constexpr auto capacity() { return size; }

private:
  struct MapEntry {
    ModuleID id;
    ModuleType type;
    void *ptr;
  };
  using Map = std::array<std::optional<MapEntry>, size>;

  Registry registry_{};
  Map map_{};
};

} // namespace sndbx

#endif