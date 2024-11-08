#include <future>
#include <iostream>

#include <mesh/node.h>

using Task = udp::Task<mesh::Node*>;

inline bool udp::Task<mesh::Node*>::await_ready() const noexcept {
  return false;
}

inline bool udp::Task<mesh::Node*>::await_suspend(std::coroutine_handle<> handle) {
  return false;
}

inline mesh::Node* udp::Task<mesh::Node*>::await_resume() {
  return m_handle.promise().m_node;
}

static Task example_usage() {
  auto node = std::make_shared<mesh::Node>(mesh::Endpoint("node1", 8080));

  /* Subscribe to events */
  node->subscribe("peer_connected", [](const std::shared_ptr<mesh::events::Event>& event) -> Task {
    auto peer_event = std::static_pointer_cast<mesh::events::Peer_connected>(event);

    log_info("Peer connected: ", peer_event->m_endpoint.to_peer_id());

    co_return;
  });

  node->subscribe("message_received", [](const std::shared_ptr<mesh::events::Event>& event) -> Task {
    auto msg_event = std::static_pointer_cast<mesh::events::Message_received>(event);

    log_info("Message received from: ", msg_event->m_from_peer);

    co_return;
  });

  co_await node->start();

  co_await node->add_peer(mesh::Endpoint("127.0.0.1", 8081));
  co_await node->add_peer(mesh::Endpoint("127.0.0.1", 8082));

  mesh::Buffer message = {'H', 'e', 'l', 'l', 'o'};

  co_await node->broadcast(message);

  co_await node->sleep_async(std::chrono::milliseconds(100));

  co_await node->stop();

  co_return;
}

int main(int argc, char** argv) {

  try {
    auto task = example_usage();

    /* Wait for completion */
    std::this_thread::sleep_for(std::chrono::seconds(65));

  } catch (const std::exception& e) {
    log_error(e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

