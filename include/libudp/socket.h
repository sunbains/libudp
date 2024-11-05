#pragma once

#include <memory>
#include <netinet/in.h>
#include <string>
#include <vector>

#include "task.h"

struct io_uring;

namespace udp {

  using Buffer = std::vector<uint8_t>;

  struct Send_operation : public IO_operation {
    Send_operation(io_uring* ring, int fd, const char* data, int n_bytes, const sockaddr_in& addr) noexcept
      : IO_operation(ring, IO_operation::Type::SEND), m_fd(fd), m_data(data), m_n_bytes(n_bytes), m_addr(addr) {

      m_iov[0].iov_len = m_n_bytes;
      m_iov[0].iov_base = const_cast<void*>(static_cast<const void*>(m_data));
    }

    void submit() override;
    int reap(io_uring_cqe* cqe) override;

    int m_fd{-1};
    msghdr m_msg_hdr{};
    sockaddr_in m_addr;
    const char* m_data;
    int m_n_bytes;
    std::array<iovec, 1> m_iov;
  };

  struct Receive_operation : public IO_operation {
    Receive_operation(io_uring* ring, int fd, Buffer& buffer) noexcept
      : IO_operation(ring, IO_operation::Type::RECEIVE), m_fd(fd), m_buffer(buffer) {

      m_iov[0].iov_len = m_buffer.size();
      m_iov[0].iov_base = m_buffer.data();

      memset(&m_client_addr, 0, sizeof(m_client_addr));
    }

    void submit() override;
    int reap(io_uring_cqe* cqe) override;

    int m_fd{-1};
    Buffer& m_buffer;
    msghdr m_msg_hdr{};
    sockaddr_in m_client_addr;
    std::array<iovec, 1> m_iov;
  };

  struct Socket {
    using IO_uring = std::unique_ptr<io_uring>;

    explicit Socket(uint16_t port);

    ~Socket();

    Task<int> receive_async(Buffer& buffer);

    Task<int> send_async(const std::string& address, uint16_t port, const char* data, int n_bytes);

    void wait_for_completion();

    void close() noexcept;

    IO_uring m_ring{};
    int m_socket_fd{-1};
    bool m_is_initialized{};
  };

}  // namespace udp
