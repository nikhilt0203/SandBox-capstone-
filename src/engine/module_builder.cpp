#include "module_builder.hpp"


auto ModuleBuilder::getEntry(sndbx::grid::Position pos) const 
  -> const ModuleBuilder::ModuleEntry*
{
  auto it = find(pos);
  if (it != m_ModuleRegistry.end()) { return it; }
  return nullptr;
}

auto ModuleBuilder::getEntry(std::uint32_t id) const 
  -> const ModuleBuilder::ModuleEntry*
{
  auto it = find(id);
  if (it != m_ModuleRegistry.end()) { return it; }
  return nullptr;
}

bool ModuleBuilder::destroy(sndbx::grid::Position pos) 
{
  auto it = std::find_if(
    m_ModuleRegistry.begin(), 
    m_ModuleRegistry.end(),
    [&pos](const auto& entry){ return entry.position == pos; }
  );

  if (it == m_ModuleRegistry.end()) { return false; }

  const auto& entry = *it;
  const auto module = entry.module;
  const auto type = entry.typeIndex;

  m_ModuleReleaseFuncs[type](module);
  m_ModuleRegistry.erase(it);
  return true;
}

auto ModuleBuilder::find(std::uint32_t id) const 
  -> ModuleBuilder::ModuleRegistry::const_iterator
{
  return std::find_if(
    m_ModuleRegistry.begin(), 
    m_ModuleRegistry.end(),
    [id](const auto& entry) { return entry.id == id; }
  );
}

auto ModuleBuilder::find(sndbx::grid::Position pos) const 
  -> ModuleBuilder::ModuleRegistry::const_iterator
{
  return std::find_if(
    m_ModuleRegistry.begin(), 
    m_ModuleRegistry.end(),
    [pos](const auto& entry){ return entry.position == pos; }
  );
}

std::uint32_t ModuleBuilder::makeModuleID() const 
{
  static std::uint32_t lastID{};
  return lastID++;
}

