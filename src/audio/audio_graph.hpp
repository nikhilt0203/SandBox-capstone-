#ifndef NST_TEENSY_AUDIO_GRAPH_HPP_
#define NST_TEENSY_AUDIO_GRAPH_HPP_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <utility>

#include "nst/expected.hpp"
#include "nst/inplace_vector.hpp"
#include "nst/object_pool.hpp"
#include "nst/strong_alias.hpp"

#include <Audio.h>

namespace nst::teensy {

// Fixed-size audio graph for Teensy Audio Library. Abstracts AudioStreams
// and AudioConnections into ID-based graph.
template <std::size_t Nodes, std::size_t Patches> class AudioGraph {
public:
  struct NodeID : public nst::strong_alias<std::uint32_t, NodeID> {
    using nst::strong_alias<std::uint32_t, NodeID>::strong_alias;

    constexpr operator bool() const { return this->value != 0; }

    constexpr bool operator==(const NodeID &other) const {
      return this->value == other.value;
    }

    constexpr bool operator!=(const NodeID &other) const {
      return this->value != other.value;
    }
  };

  struct Node {
    NodeID id;
    ::AudioStream *device;
    Node(NodeID id, ::AudioStream *device) : id{id}, device{device} {}
  };

  template <class AudioDevice> struct NodeHandle {
    NodeID id;
    AudioDevice *device;
    const AudioDevice *operator->() const { return device; }
    AudioDevice *operator->() { return device; }
  };

  struct Port : public nst::strong_alias<std::uint8_t, Port> {
    using nst::strong_alias<std::uint8_t, Port>::strong_alias;

    constexpr bool operator==(const Port &other) const {
      return this->value == other.value;
    }

    constexpr bool operator!=(const Port &other) const {
      return this->value != other.value;
    }

    [[nodiscard]] constexpr auto operator*() const { return this->value; }
  };

  struct Patch {
    NodeID src_id;
    Port src_port;
    NodeID dst_id;
    Port dst_port;
    ::AudioConnection *connection;

    Patch(Node src, Port src_port, Node dst, Port dst_port,
          ::AudioConnection *c)
        : src_id{src.id}, src_port{src_port}, dst_id{dst.id},
          dst_port{dst_port}, connection{c} {
      c->connect(*src.device, *src_port, *dst.device, *dst_port);
    }
  };

  enum class Error {
    NONE,
    POOL_EXHAUSTED,
    PATCH_ALREADY_EXISTS,
    NODE_NOT_FOUND,
    PATCH_NOT_FOUND,
    GRAPH_FULL,
    INVALID_PORT
  };

public:
  auto add_node(::AudioStream *device) -> nst::expected<NodeID, Error> {
    if (nodes_.is_full()) {
      return Error::GRAPH_FULL;
    }
    return emplace_and_get_id(device);
  }

  template <class T, typename... Args>
  auto emplace_node(Args &&...args) -> nst::expected<NodeHandle<T>, Error> {
    static_assert(std::is_base_of_v<::AudioStream, T>);
    if (nodes_.is_full()) {
      return Error::GRAPH_FULL;
    }
    const auto device = new T(std::forward<Args>(args)...);
    const auto id = emplace_and_get_id(device);
    return NodeHandle<T>{id, device};
  }

  bool remove_node(NodeID id) {
    auto it = find_node(id);
    if (it == nodes_.end()) {
      return false;
    }
    nodes_.erase(it);
    patches_.erase(std::remove_if(patches_.begin(), patches_.end(),
                                  [id](const auto &p) {
                                    return p.src_id == id || p.dst_id == id;
                                  }),
                   patches_.end());
    return true;
  }

  auto connect(NodeID src_id, Port src_port, NodeID dst_id, Port dst_port)
      -> Error {
    if (!(connection_pool_.num_active() < connection_pool_.size()) ||
        patches_.is_full()) {
      return Error::POOL_EXHAUSTED;
    }

    if (find_patch(src_id, src_port, dst_id, dst_port) != patches_.end()) {
      return Error::PATCH_ALREADY_EXISTS;
    }

    const auto src = find_node(src_id);
    const auto dst = find_node(dst_id);
    if (src == nodes_.end() || dst == nodes_.end()) {
      return Error::NODE_NOT_FOUND;
    }

    emplace_patch(*src, src_port, *dst, dst_port);
    return Error::NONE;
  }

  auto disconnect(NodeID src_id, Port src_port, NodeID dst_id, Port dst_port)
      -> Error {
    auto it = find_patch(src_id, src_port, dst_id, dst_port);
    if (it == patches_.end()) {
      return Error::PATCH_NOT_FOUND;
    }
    release_patch(*it);
    patches_.erase(it);
    return Error::NONE;
  }

  auto disconnect(NodeID src_id, NodeID dst_id) -> Error {
    const auto begin = patches_.begin();
    const auto end = patches_.end();

    auto it = std::partition(begin, end, [src_id, dst_id](const auto &p) {
      return p.src_id == src_id && p.dst_id == dst_id;
    });
    if (it == begin) {
      return Error::PATCH_NOT_FOUND;
    }

    std::for_each(begin, it, [](auto &p) { release_patch(p); });
    patches_.erase(it, end);
    return Error::NONE;
  }

  [[nodiscard]] AudioStream &operator[](NodeID id) {
    auto it = find_node(id);
    assert(it != nodes_.end());
    return *it->device;
  }

  [[nodiscard]] const AudioStream &operator[](NodeID id) const {
    auto it = find_node(id);
    assert(it != nodes_.end());
    return *it->device;
  }

  [[nodiscard]] const auto &nodes() const { return nodes_; }
  [[nodiscard]] const auto &patches() const { return patches_; }
  [[nodiscard]] auto &nodes() { return nodes_; }
  [[nodiscard]] auto &patches() { return patches_; }

private:
  void emplace_patch(Node src, Port src_port, Node dst, Port dst_port) {
    const auto c = connection_pool_.acquire();
    assert(c);
    patches_.emplace_back(src, src_port, dst, dst_port, c);
  }

  void release_patch(Patch &p) {
    p.connection->disconnect();
    connection_pool_.release(p.connection);
  }

  auto find_node(NodeID id) {
    return std::find_if(nodes_.begin(), nodes_.end(),
                        [id](const auto &n) { return n.id == id; });
  }

  auto find_node(NodeID id) const {
    return std::find_if(nodes_.cbegin(), nodes_.cend(),
                        [id](const auto &n) { return n.id == id; });
  }

  auto find_patch(NodeID src_id, Port src_port, NodeID dst_id,
                  Port dst_port) const {
    return std::find_if(patches_.cbegin(), patches_.cend(), [&](const auto &p) {
      return src_id == p.src_id && src_port == p.src_port &&
             dst_id == p.dst_id && dst_port == p.dst_port;
    });
  }

  auto find_patch(NodeID src_id, NodeID dst_id) const {
    return std::find_if(patches_.cbegin(), patches_.end(), [&](const auto &p) {
      return src_id == p.src_id && dst_id == p.dst_id;
    });
  }

  auto find_patch(NodeID src_id, Port src_port, NodeID dst_id, Port dst_port) {
    return std::find_if(patches_.begin(), patches_.end(),
                        [src_id, src_port, dst_id, dst_port](const auto &p) {
                          return src_id == p.src_id && src_port == p.src_port &&
                                 dst_id == p.dst_id && dst_port == p.dst_port;
                        });
  }

  NodeID emplace_and_get_id(::AudioStream *device) {
    const auto new_id = node_id_gen_.next_id();
    nodes_.emplace_back(new_id, device);
    return new_id;
  }

  struct {
    NodeID next_id() {
      ++last_id.value;
      return NodeID{last_id};
    }
    NodeID last_id{};
  } node_id_gen_;

  nst::inplace_vector<Node, Nodes> nodes_;
  nst::inplace_vector<Patch, Patches> patches_;
  nst::object_pool<::AudioConnection, Patches> connection_pool_;
};

} // namespace nst::teensy

#endif