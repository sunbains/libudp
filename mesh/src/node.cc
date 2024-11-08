#include <future>

#include "mesh/node.h"

namespace mesh {

Node::Node(const Endpoint& endpoint)
  : m_endpoint(endpoint),
    m_socket(endpoint.m_port),
    m_dispatcher(std::make_shared<events::Dispatcher>()),
    m_running() { }

udp::Task<Node*> Node::start() {
  if (m_running) {
    co_return;
  }

  m_running = true;

  /* Start background tasks */
  m_receive_task = receive_loop();
  m_health_check_task = health_check_loop();
  m_discovery_task = discovery_loop();

  /* Notify network state change */
  auto event = std::make_shared<events::Network_state_changed>();

  event->m_is_healthy = true;
  event->m_status = "Node started";

  co_await m_dispatcher->dispatch(event);

  co_return;
}

udp::Task<Node*> Node::stop() {
  if (!m_running) {
    co_return;
  }

  m_running = false;

  /* Notify network state change */
  auto event = std::make_shared<events::Network_state_changed>();

  event->m_is_healthy = false;
  event->m_status = "Node stopped";

  co_await m_dispatcher->dispatch(event);

  co_return;
}

udp::Task<Node*> Node::send_to_peer(const Endpoint &endpoint, const Buffer& buffer) {
  std::shared_lock lock(m_peers_mutex);

  if (auto it = m_peers.find(endpoint.to_peer_id()); it != m_peers.end() && it->second.m_is_active) {

    const auto port{it->second.m_endpoint.m_port};
    const auto &address{it->second.m_endpoint.m_address};

    auto task = m_socket.send_async(address, port, buffer.data(), buffer.size());

    if (!task.is_done()) {
      // FIXME: Schedule another task to resum. This makes it synchronous.
      task.resume();
    }

    co_return;
  }

  co_return;
}

udp::Task<Node*> Node::broadcast(const Buffer& buffer) {
  std::shared_lock lock(m_peers_mutex);

  for (const auto& [peer_id, peer] : m_peers) {
    if (peer.m_is_active) {

      const auto port{peer.m_endpoint.m_port};
      const auto address{peer.m_endpoint.m_address};

      auto task = m_socket.send_async(address, port, buffer.data(), buffer.size());

      if (!task.is_done()) {
        // FIXME: Schedule another task to resum. This makes it synchronous.
        task.resume();
      }
    }
  }
  co_return;
}

udp::Task<Node*> Node::add_peer(const Endpoint &endpoint) {
    std::unique_lock lock(m_peers_mutex);
    std::string peer_id{endpoint.to_peer_id()};

    if (m_peers.contains(peer_id)) {
      co_return;
    }

    Peer peer;

    peer.m_is_active = true;
    peer.m_endpoint = endpoint;
    peer.m_last_seen = std::chrono::steady_clock::now();

    m_peers[peer_id] = peer;

    auto event = std::make_shared<events::Peer_connected>();

    event->m_endpoint = endpoint;

    co_await m_dispatcher->dispatch(event);

    co_return;
  }

void Node::subscribe(const std::string& event_type, events::Handler handler) {
  m_dispatcher->subscribe(event_type, std::move(handler));
}

udp::Task<Node*> Node::receive_loop() {
  /* Max size of a UDP packet */
  Buffer buffer(65536);

  std::string error_message{};

  while (m_running) {
    try {
      auto task = m_socket.receive_async(buffer);

      if (!task.is_done()) {
        // FIXME: Schedule another task to resum. This makes it synchronous.
        task.resume();
      }

      /* Create message received event */
      auto event = std::make_shared<events::Message_received>();
      event->m_data = buffer;

      co_await m_dispatcher->dispatch(event);

    } catch (const std::exception& e) {
      error_message = std::string("Receive error: ") + e.what();
    }

    if (!error_message.empty()) {
      auto event = std::make_shared<events::Network_state_changed>();

      event->m_is_healthy = false;
      event->m_status = error_message;

      co_await m_dispatcher->dispatch(event);
    }
  }
}

udp::Task<Node*> Node::health_check_loop() {
  std::string error_message{};

  while (m_running) {
    try {
      std::unique_lock lock(m_peers_mutex);
      auto now = std::chrono::steady_clock::now();

      for (auto it = m_peers.begin(); it != m_peers.end();) {
        if (now - it->second.m_last_seen > std::chrono::seconds(30)) {
          if (it->second.m_is_active) {
            it->second.m_is_active = false;

            // Notify peer disconnected
            auto event = std::make_shared<events::Peer_disconnected>();

            event->m_peer_id = it->first;

            co_await m_dispatcher->dispatch(event);
          }
          it = m_peers.erase(it);
        } else {
          ++it;
        }
      }
    } catch (const std::exception& e) {
      error_message = std::string("Health check error: ") + e.what();
    }

    if (!error_message.empty()) {
      auto event = std::make_shared<events::Network_state_changed>();

      event->m_is_healthy = false;
      event->m_status = error_message;

      co_await m_dispatcher->dispatch(event);
    }

    auto event = std::make_shared<events::Sleep_for>();

    event->m_duration = std::chrono::seconds(5);

    co_await m_dispatcher->dispatch(event);
  }
}

udp::Task<Node*> Node::discovery_loop() {
  while (m_running) {
    std::string error_message{};

    try {

      Buffer msg = {'D', 'I', 'S', 'C'};

      co_await broadcast(msg);

    } catch (const std::exception& e) {
      error_message = std::string("Discovery error: ") + e.what();
    }

    if (!error_message.empty()) {
      auto event = std::make_shared<events::Network_state_changed>();

      event->m_is_healthy = false;
      event->m_status = error_message;

      co_await m_dispatcher->dispatch(event);
    }

    auto event = std::make_shared<events::Sleep_for>();

    event->m_duration = std::chrono::seconds(30);

    co_await m_dispatcher->dispatch(event);
  }
}

udp::Task<Node*> Node::sleep_async(std::chrono::milliseconds duration) {
  auto event = std::make_shared<events::Sleep_for>();

  event->m_duration = duration;

  co_await m_dispatcher->dispatch(event);
}

} // namespace mesh
