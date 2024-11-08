#pragma once

#include <cstdint>
#include <string>
#include <chrono>

#include <libudp/socket.h>

namespace mesh {

using Buffer = udp::Buffer;

struct Endpoint {
  Endpoint() = default;
  ~Endpoint() = default;

  /**
   * @brief Construct an endpoint from an address and port
   */
  Endpoint(const std::string& address, uint16_t port)
    : m_address(address), m_port(port) {}

  std::string to_string() const noexcept {
    return "{ m_address: " + m_address + ", port: " + std::to_string(m_port) + " }";
  }

  bool operator==(const Endpoint& rhs) const {
    return m_port == rhs.m_port && m_address == rhs.m_address;
  }

  bool is_valid() const noexcept {
    return m_port > 0 && !m_address.empty();
  }

  std::string to_peer_id() const noexcept {
    return m_address + ":" + std::to_string(m_port);
  }

  static Endpoint from_peer_id(const std::string& peer_id) noexcept{
    size_t colon_pos = peer_id.find(':');

    if (colon_pos != std::string::npos) {

      std::string number_part = peer_id.substr(colon_pos + 1);

      try {
        return Endpoint(peer_id.substr(0, colon_pos), uint16_t(std::stoi(number_part)));
      } catch (const std::invalid_argument& e) {
        log_error("Invalid number format: ", e.what());
      } catch (const std::out_of_range& e) {
        log_error("Number out of range: ", e.what());
      }
    }

    log_error("Colon not found in the peer id: ", peer_id);

    return Endpoint("", 0);
  }

  uint16_t m_port{};
  std::string m_address{};
};

struct Peer {

  bool operator==(const Peer& rhs) const {
    return m_endpoint == rhs.m_endpoint;
  }

  std::string to_string() const noexcept {
    return"m_is_active: " + std::to_string(m_is_active) +
	    ", m_endpoint: " + m_endpoint.to_string() +
	    ", m_last_seen: " + std::to_string(m_last_seen.time_since_epoch().count());
  }

  /* Round trip time in milliseconds */
  bool m_is_active{};
  double m_rtt_ms{0.0};
  Endpoint m_endpoint{};
  std::chrono::steady_clock::time_point m_last_seen{};
};

} // namespace mesh

