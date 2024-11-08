#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "libudp/task.h"
#include "mesh/peer.h"
#include "mesh/task.h"

namespace mesh::events {

struct Event {
  virtual ~Event() = default;
  virtual std::string get_type() const = 0;
};

struct Peer_connected : public Event {
  std::string get_type() const override {
    return "Peer_connected";
  }

  Endpoint m_endpoint;
};

struct Peer_disconnected : public Event {
  std::string get_type() const override {
    return "Peer_disconnected";
  }

  std::string m_peer_id;
};

struct Message_received : public Event {
  std::string get_type() const override {
    return "Message_received";
  }

  Buffer m_data;
  std::string m_from_peer;
};

struct Network_state_changed : public Event {
  std::string get_type() const override {
    return "Network_state_changed";
  }

  bool m_is_healthy;
  std::string m_status;
};

struct Sleep_for : public Event {
  std::string get_type() const override {
    return "Sleep_for";
  }

  std::chrono::milliseconds m_duration;
};

struct Dispatcher;

using Handler = std::function<udp::Task<Node*>(const std::shared_ptr<Event>&)>;

struct Dispatcher {
  void subscribe(const std::string& event_type, Handler handler) {
    m_handlers[event_type].push_back(std::move(handler));
  }

  udp::Task<Node*> dispatch(std::shared_ptr<Event>&& event) {
    if (auto it = m_handlers.find(event->get_type()); it != m_handlers.end()) {
      for (const auto& handler : it->second) {
        co_await handler(std::move(event));
      }
    }
  }

  std::unordered_map<std::string, std::vector<Handler>> m_handlers{};
};

} // namespace mesh::events
