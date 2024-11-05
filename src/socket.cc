#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "libnet/socket.h"

namespace udp_library {

void Send_operation::submit_operation() {
    auto  sqe = io_uring_get_sqe(m_ring);

    if (sqe == nullptr) {
        throw std::runtime_error("Failed to get SQE");
    }

    io_uring_prep_sendto(sqe, m_fd, m_data.data(), m_data.size(), 0, reinterpret_cast<const sockaddr*>(&m_addr), sizeof(m_addr));

    io_uring_sqe_set_data(sqe, this);

    io_uring_submit(m_ring);
}

void Receive_operation::submit_operation() {
    auto sqe = io_uring_get_sqe(m_ring);

    if (sqe == nullptr) {
        throw std::runtime_error("Failed to get SQE");
    }

    io_uring_prep_recvfrom(sqe, m_fd, m_buffer.data(), m_buffer.size(), 0, reinterpret_cast<sockaddr*>(&m_client_addr), &m_client_addr_len);

    io_uring_sqe_set_data(sqe, this);

    io_uring_submit(m_ring);
}

Socket::Socket(uint16_t port) : m_socket_fd(-1), m_is_initialized(false) {
    m_socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    if (m_socket_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close();
        throw std::runtime_error("Failed to bind socket");
    }

    if (io_uring_queue_init(32, &m_ring, 0) < 0) {
        close();
        throw std::runtime_error("Failed to initialize io_uring");
    }

    // Start the completion processing thread
    m_is_initialized = true;
}

Socket::~Socket() {
    close();
}

Task<size_t> Socket::send_async(const std::string& address, uint16_t port, const std::vector<uint8_t>& data) {
    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address");
    }

    Send_operation op(&m_ring, m_socket_fd, data, addr);

    co_return co_await op;
}

Task<size_t> Socket::receive_async(std::vector<uint8_t>& buffer) {
    Receive_operation op(&m_ring, m_socket_fd, buffer);

    co_return co_await op;
}

void Socket::close() {
    if (m_is_initialized) {
        io_uring_queue_exit(&m_ring);
    }

    if (m_socket_fd >= 0) {
        ::close(m_socket_fd);
        m_socket_fd = -1;
    }

    m_is_initialized = false;
}

}
