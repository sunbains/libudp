#include "libudp/socket.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include <liburing.h>

namespace udp {

  bool IO_operation::await_ready() const noexcept {
    const auto ready = io_uring_cq_ready(m_ring) > 0;
    log_debug(type(), " completion queue is ", ready ? "ready" : "not ready");
    return ready;
  }

  void Send_operation::submit() {
    auto sqe = io_uring_get_sqe(m_ring);

    if (sqe == nullptr) {
      throw std::runtime_error("Failed to get SQE");
    }

    sqe->flags |= IOSQE_ASYNC;

    std::memset(&m_msg_hdr, 0, sizeof(m_msg_hdr));

    m_msg_hdr.msg_iov = m_iov.data();
    m_msg_hdr.msg_iovlen = m_iov.size();
    m_msg_hdr.msg_namelen = sizeof(m_addr);
    m_msg_hdr.msg_name = reinterpret_cast<void*>(&m_addr);

    io_uring_prep_sendmsg(sqe, m_fd, &m_msg_hdr, 0);

    io_uring_sqe_set_data(sqe, this);

    if (is_log_level(Logger::Level::DEBUG)) [[unlikely]] {
      std::string addr{inet_ntoa(m_addr.sin_addr)};

      addr.push_back(':');
      addr.append(std::to_string(ntohs(m_addr.sin_port)));
    }

    auto ret = io_uring_submit(m_ring);

    if (ret < 0) {
      log_error("Failed to submit operation: ", strerror(ret));
      throw std::runtime_error("Failed to submit operation");
    }
  }

  int Send_operation::reap(io_uring_cqe* cqe) {
    const auto ret = cqe->res;
    assert(ret != -EAGAIN && ret != -EINTR);

    return ret;
  }

  void Receive_operation::submit() {
    auto sqe = io_uring_get_sqe(m_ring);

    if (sqe == nullptr) {
      throw std::runtime_error("Failed to get SQE");
    }

    assert(m_fd >= 0);
    assert(m_buffer.size() > 0);
    assert(m_buffer.data() != nullptr);

    sqe->flags |= IOSQE_ASYNC;

    std::memset(&m_msg_hdr, 0, sizeof(m_msg_hdr));

    m_msg_hdr.msg_iov = m_iov.data();
    m_msg_hdr.msg_iovlen = m_iov.size();
    m_msg_hdr.msg_namelen = sizeof(m_client_addr);
    m_msg_hdr.msg_name = reinterpret_cast<void*>(&m_client_addr);

    io_uring_prep_recvmsg(sqe, m_fd, &m_msg_hdr, 0);

    io_uring_sqe_set_data(sqe, this);

    if (is_log_level(Logger::Level::DEBUG)) [[unlikely]] {
      std::string addr{inet_ntoa(m_client_addr.sin_addr)};

      addr.push_back(':');
      addr.append(std::to_string(ntohs(m_client_addr.sin_port)));
    }

    int ret;

    while ((ret = io_uring_submit_and_wait(m_ring, 1)) == -EAGAIN || ret == -EINTR) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (ret < 0) {
      throw std::runtime_error("Failed to submit operation");
    }
  }

  int Receive_operation::reap(io_uring_cqe *cqe) {
    const auto ret = cqe->res;
    assert(ret != -EAGAIN && ret != -EINTR);

    return ret;
  }

  Socket::Socket(uint16_t port) : m_socket_fd(-1), m_is_initialized(false) {

    m_ring = std::make_unique<io_uring>();

    m_socket_fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);

    if (m_socket_fd < 0) {
      throw std::runtime_error("Failed to create socket");
    }

    {
      int val{1};

      setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
      setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    }

    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    log_info("Binding socket to port " + std::to_string(port));

    if (::bind(m_socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      close();
      throw std::runtime_error("Failed to bind socket");
    }

    {
      int ret = io_uring_queue_init(32, m_ring.get(), 0);

      if (ret < 0) {
        close();
        throw std::runtime_error(std::string("Failed to initialize io_uring: ") + strerror(-ret));
      }
    }

    /* Start the completion processing thread. */
    m_is_initialized = true;
  }

  Socket::~Socket() {
    close();
  }

  Task<int> Socket::send_async(const std::string& address, uint16_t port, const char* data, int n_bytes) {
    sockaddr_in addr{};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
      throw std::runtime_error("Invalid address");
    }

    Send_operation op(m_ring.get(), m_socket_fd, data, n_bytes, addr);

    co_return co_await op;
  }

  Task<int> Socket::receive_async(Buffer& buffer) {
    Receive_operation op(m_ring.get(), m_socket_fd, buffer);

    co_return co_await op;
  }

  void Socket::wait_for_completion() {
    int ret;
    io_uring_cqe* cqe;

    while ((ret = io_uring_wait_cqe(m_ring.get(), &cqe)) == -EAGAIN || ret == -EINTR) {
      log_debug("Waiting for completion queue to be ready...");
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (ret < 0) [[unlikely]] {
      throw std::runtime_error(std::string("Failed to wait for completion: ") + strerror(-ret));
    }

    if (auto io_operation = static_cast<IO_operation*>(io_uring_cqe_get_data(cqe)); io_operation != nullptr) [[likely]] {
      log_debug(io_operation->type(), " completion queue event: ret: ", abs(cqe->res));
      const auto ret = io_operation->reap(cqe);
      io_operation->complete(ret);
    }

    io_uring_cqe_seen(m_ring.get(), cqe);
  }

  void Socket::close() noexcept {
    if (m_is_initialized) {
      io_uring_queue_exit(m_ring.get());
    }

    if (m_socket_fd >= 0) {
      ::close(m_socket_fd);
      m_socket_fd = -1;
    }

    m_is_initialized = false;
  }

}  // namespace udp
