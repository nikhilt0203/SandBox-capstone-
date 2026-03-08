#ifndef SANDBOX_AUDIO_GRAPH_HPP_
#define SANDBOX_AUDIO_GRAPH_HPP_

#include <Audio.h>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <cassert>
#include "core/object_pool.hpp"
#include "core/fixed_vector.hpp"

//=====================================================
// Holds AudioStream objects and an I/O map
//=====================================================
class AudioComponent
{
public:
  struct AudioStreamPort
  {
    AudioStream* device;
    std::size_t port;
  };

  static constexpr std::size_t maxPorts = 8;
  using PortMap = std::array<std::optional<AudioStreamPort>, maxPorts>;
  
public:
  AudioComponent() = default;

  template<typename T>
  void addDevice() 
  { 
    static_assert(std::is_base_of_v<AudioStream, T>);
    __disable_irq();
    m_AudioDevices.emplace_back(std::make_unique<T>()); 
    __enable_irq();
  }

  template<typename T>
  [[nodiscard]] T* device(std::size_t index = 0) const
  {
    static_assert(std::is_base_of_v<AudioStream, T>);
    return static_cast<T*>(m_AudioDevices.at(index).get());
  }

  void mapInput(std::size_t index, AudioStream* device, std::size_t devicePort) 
  { 
    m_InputMap[index] = AudioStreamPort{device, devicePort}; 
  }

  void mapOutput(std::size_t index, AudioStream* device, std::size_t devicePort) 
  { 
    m_OutputMap[index] = AudioStreamPort{device, devicePort}; 
  }

  [[nodiscard]] AudioStreamPort getInputMapping(std::size_t index) const { return *m_InputMap[index]; }
  [[nodiscard]] AudioStreamPort getOutputMapping(std::size_t index) const { return *m_OutputMap[index]; }

  [[nodiscard]] const PortMap& inputMap() const { return m_InputMap; }
  [[nodiscard]] const PortMap& outputMap() const { return m_OutputMap; }

  [[nodiscard]] float processorUsage() const
  {
    float total{};
    for (const auto& device: m_AudioDevices) { total += device->processorUsage(); }
    return total;
  }

private:
  sndbx::vector_8U<std::unique_ptr<AudioStream>> m_AudioDevices;
  PortMap m_InputMap{};
  PortMap m_OutputMap{};
};

class Patchable
{
public:
  Patchable() = default;
  virtual ~Patchable() = default;

  [[nodiscard]] AudioComponent& audio() { return m_Audio; }
  [[nodiscard]] const AudioComponent& audio() const { return m_Audio; }

protected:
  AudioComponent m_Audio;
};

class AudioGraph
{
public:
  struct Patch
  {
    Patchable* source{};
    std::size_t sourcePort{};
    Patchable* destination{};
    std::size_t destinationPort{};
    AudioConnection m_Connection;

    Patch() = default;

    void connect(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) 
    {
      auto srcOut = src->audio().getOutputMapping(srcPort);
      auto destIn = dest->audio().getInputMapping(destPort);

      m_Connection.connect(
        *(srcOut.device), 
        srcOut.port, 
        *(destIn.device), 
        destIn.port
      );

      source = src;
      sourcePort = srcPort;
      destination = dest;
      destinationPort = destPort;
    }

    void disconnect() 
    {
      m_Connection.disconnect();
      source = nullptr;
      destination = nullptr;
    }

    [[nodiscard]] bool isSame(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) const
    {  
      return source == src && 
             sourcePort == srcPort && 
             destination == dest && 
             destinationPort == destPort;
    }
  };

public:
  AudioGraph() { AudioMemory(100); }

  bool connect(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
  {
    if (src == dest) { return false; }

    if (!portExists(src->audio().outputMap(), srcPort)) { return false; }
    if (!portExists(dest->audio().inputMap(), destPort)) { return false; }

    if (patchExists(src, srcPort, dest, destPort)) { return false; }

    auto patch = m_PatchPool.acquire();
    if (!patch) { return false; }

    if (!m_Patches.push_back(patch))
    {
      m_PatchPool.release(patch);
      return false;
    }

    patch->connect(src, srcPort, dest, destPort);
    Serial.println(m_Patches.size());
    return true;
  }

  bool disconnect(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
  {
    if (src == dest) { return false; }

    auto it = findPatch(src, srcPort, dest, destPort);
    if (it == m_Patches.end()) { return false; }

    auto patch = *it;

    patch->disconnect();
    
    m_PatchPool.release(patch);
    m_Patches.erase(it);
    Serial.println(m_Patches.size());
    return true;
  }

  void deletePatchesWith(Patchable* node)
  {
    if (!node) { return; }

    auto removePatch = 
      [this, node](Patch* patch) {
        if (patch->destination == node || patch->source == node)
        {
          patch->disconnect();
          m_PatchPool.release(patch);
          return true;
        }
        return false;
      };

    m_Patches.erase(std::remove_if(m_Patches.begin(), m_Patches.end(), removePatch));
  }

  [[nodiscard]] bool patchExists(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) const
  {
    return findPatch(src, srcPort, dest, destPort) != m_Patches.end();
  }

  void clear() 
  { 
    for (auto patch : m_Patches)
    {
      patch->disconnect();
      m_PatchPool.release(patch);
    }
    m_Patches.clear();
  }

  float processorUsage() const { return AudioProcessorUsage(); } 

private:
  [[nodiscard]] Patch* const* findPatch(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) const
  {
    return std::find_if(
      m_Patches.begin(), 
      m_Patches.end(),
      [src, srcPort, dest, destPort](const Patch* patch){
        return patch->isSame(src, srcPort, dest, destPort);
      });
  }

  [[nodiscard]] Patch** findPatch(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
  {
    return std::find_if(
      m_Patches.begin(), 
      m_Patches.end(),
      [src, srcPort, dest, destPort](const Patch* patch){
        return patch->isSame(src, srcPort, dest, destPort);
      });
  }

  [[nodiscard]] bool portExists(const AudioComponent::PortMap& portMap, std::size_t port) const noexcept 
  {
    if (port >= portMap.size()) { return false; }
    return portMap[port].has_value();
  }

private:
  static constexpr std::size_t maxPatches = 64;
  sndbx::object_pool<Patch, maxPatches> m_PatchPool;
  sndbx::fixed_vector<Patch*, maxPatches> m_Patches;
  inline static AudioOutputI2S i2s{}; //needed to start audio interrupts
};

#endif