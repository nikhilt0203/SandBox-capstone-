#ifndef SANDBOX_AUDIO_GRAPH_HPP_
#define SANDBOX_AUDIO_GRAPH_HPP_

#include <Audio.h>
#include <vector>
#include <map>
#include <memory>

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

  using PortMap = std::map<std::size_t, AudioStreamPort>;
  
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

  [[nodiscard]] AudioStreamPort getInputMapping(std::size_t index) const { return m_InputMap.at(index); }
  [[nodiscard]] AudioStreamPort getOutputMapping(std::size_t index) const { return m_OutputMap.at(index); }

  [[nodiscard]] const PortMap& inputMap() const { return m_InputMap; }
  [[nodiscard]] const PortMap& outputMap() const { return m_OutputMap; }

  [[nodiscard]] float processorUsage() const
  {
    float total{};
    for (const auto& device: m_AudioDevices) { total += device->processorUsage(); }
    return total;
  }

private:
  std::vector<std::unique_ptr<AudioStream>> m_AudioDevices;
  PortMap m_InputMap;
  PortMap m_OutputMap;
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
    std::unique_ptr<AudioConnection> m_Connection;
    Patchable* m_Source;
    std::size_t m_SourcePort;
    Patchable* m_Destination;
    std::size_t m_DestinationPort;

    Patch(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
    : m_Connection(std::make_unique<AudioConnection>(
        *(src->audio().getOutputMapping(srcPort).device), 
          src->audio().getOutputMapping(srcPort).port, 
        *(dest->audio().getInputMapping(destPort).device), 
          dest->audio().getInputMapping(destPort).port
      )),
      m_Source(src), 
      m_SourcePort(srcPort), 
      m_Destination(dest), 
      m_DestinationPort(destPort) 
    {}
  };

public:
  AudioGraph() { AudioMemory(100); }

  bool connect(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
  {
    if (src == dest) { return false; }

    if (!portExists(src->audio().outputMap(), srcPort)) { return false; }
    if (!portExists(dest->audio().inputMap(), destPort)) { return false; }

    if (patchExists(src, srcPort, dest, destPort)) { return false; }

    __disable_irq();
    m_Connections.emplace_back(src, srcPort, dest, destPort);
    __enable_irq();
    return true;
  }

  bool disconnect(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort)
  {
    if (src == dest) { return false; }

    auto it = findPatch(src, srcPort, dest, destPort);
    
    if (it == m_Connections.end()) { return false; }
    
    __disable_irq();
    m_Connections.erase(it);
    __enable_irq();
    return true;
  }

  bool deletePatchesWith(Patchable* node)
  {
    if (!node) { return false; }

    auto it = std::find_if(
      m_Connections.begin(), 
      m_Connections.end(),
      [node](const Patch& patch){
        return patch.m_Destination == node ||
               patch.m_Source == node;
      });
    
    if (it == m_Connections.end()) { return false; }
    
    __disable_irq();
    m_Connections.erase(it);
    __enable_irq();
    return true;
  }

  [[nodiscard]] bool patchExists(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) const
  {
    return findPatch(src, srcPort, dest, destPort) != m_Connections.end();
  }

  void clear() 
  { 
    __disable_irq();
    m_Connections.clear(); 
    __enable_irq();
  }

  float processorUsage() const { return AudioProcessorUsage(); } 

private:
  [[nodiscard]] std::vector<Patch>::const_iterator findPatch(Patchable* src, std::size_t srcPort, Patchable* dest, std::size_t destPort) const
  {
    return std::find_if(
      m_Connections.begin(), 
      m_Connections.end(),
      [src, srcPort, dest, destPort](const Patch& patch){
        return patch.m_Source == src && 
               patch.m_SourcePort == srcPort && 
               patch.m_Destination == dest && 
               patch.m_DestinationPort == destPort;
      });
  }

  [[nodiscard]] bool portExists(const AudioComponent::PortMap& portMap, std::size_t port) const
  {
    return portMap.find(port) != portMap.end();
  }

private:
  std::vector<Patch> m_Connections{};
  inline static AudioOutputI2S i2s{}; //needed to start audio interrupts
};

#endif