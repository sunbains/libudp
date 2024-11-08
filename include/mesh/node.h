#pragma once

#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "libudp/socket.h"
#include "mesh/events.h"

namespace mesh {

struct Node : public std::enable_shared_from_this<Node> {

  explicit Node(const Endpoint& endpoint);

  udp::Task<Node*> start();

  udp::Task<Node*> stop();

  udp::Task<Node*> send_to_peer(const Endpoint& endpoint, const Buffer& buffer);

  udp::Task<Node*> broadcast(const Buffer& buffer);

  udp::Task<Node*> add_peer(const Endpoint &endpoint);

  void subscribe(const std::string& event_type, events::Handler handler);

  udp::Task<Node*> sleep_async(std::chrono::milliseconds duration);

private:
  /* Listen for incoming messages */
  udp::Task<Node*> receive_loop();

  udp::Task<Node*> health_check_loop();

  udp::Task<Node*> discovery_loop();

private:
  Endpoint m_endpoint{};
  udp::Socket m_socket;
  std::atomic<bool> m_running;
  std::shared_ptr<events::Dispatcher> m_dispatcher;

  mutable std::shared_mutex m_peers_mutex;

  std::unordered_map<std::string, Peer> m_peers;

  udp::Task<Node*> m_receive_task;
  udp::Task<Node*> m_health_check_task;
  udp::Task<Node*> m_discovery_task;
};

} // namespace mesh
