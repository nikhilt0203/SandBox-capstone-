#ifndef SANDBOX_OBJECT_POOL_HPP_
#define SANDBOX_OBJECT_POOL_HPP_

#include <array>

namespace sndbx 
{

template<typename T, std::size_t N>
struct object_pool
{
  struct entry
  {
    T object;
    bool active{};
  };

  std::array<entry, N> entries;

  T* acquire()
  {
    for (auto& entry : entries)
    {
      if (!entry.active) 
      {
        entry.active = true;
        return &(entry.object);
      }
    }
    return nullptr;
  }

  void release(T* obj)
  {
    for (auto& entry : entries)
    {
      if (&(entry.object) == obj) 
      {
        entry.active = false;
        return;
      }
    }
  }
};

}

#endif