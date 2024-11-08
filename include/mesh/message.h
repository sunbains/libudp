#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <string>

#include <mesh/peer.h>

namespace mesh {

enum class Message_type : uint8_t {
  None = 0,
  Discovery = 1,
  Discovery_response = 2,
  Peer_list = 3,
  Data = 4,
  Heartbeat = 5
};

using Message_ttl = uint16_t;
using Message_id = uint64_t;

inline std::string to_string(const Message_type type) noexcept {
  switch (type) {
    case Message_type::None:
      return "None";
    case Message_type::Discovery:
      return "Discovery";
    case Message_type::Discovery_response:
      return "Discovery_response";
    case Message_type::Peer_list:
      return "Peer_list";
    case Message_type::Data:
      return "Data";
    case Message_type::Heartbeat:
      return "Heartbeat";
    default:
     std::terminate();
  }
}

struct Message_header {
  std::string to_string() const noexcept {
    return "m_type : " + mesh::to_string(m_type) +
            ", m_message_id: " + std::to_string(m_message_id) +
            ", m_source_id: " + m_source_id +
            ", m_ttl: " + std::to_string(m_ttl);
  }

  Message_type m_type{Message_type::None};;
  Message_id m_message_id{};
  std::string m_source_id{};
  Message_ttl m_ttl{};
};

struct Message {
  std::string to_string() const noexcept {
    return "m_header: { " + m_header.to_string() + " }" +
           ", m_payload_size: " + std::to_string(m_payload.size()) +
           ", m_payload: TBD";
  }

  Message_header m_header;
  Buffer m_payload;
};

struct Serialize {
  Buffer operator()(const Message& msg) const noexcept;
};

struct Deserialize {
   Message operator()(const udp::Buffer& buffer) const noexcept;
};

} // namespace mesh
