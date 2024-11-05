#pragma once

#include <liburing.h>
#include <string>
#include <vector>

#include "task.h"

namespace udp {

class Send_operation : public IO_operation {
public:
    Send_operation(io_uring* ring, int fd, const std::vector<uint8_t>& data, const sockaddr_in& addr)
        : IO_operation(ring), m_fd(fd), m_data(data), m_addr(addr) {}

protected:
    void submit_operation() override;

private:
    int m_fd;
    sockaddr_in m_addr;
    const std::vector<uint8_t>& m_data;
};

class Receive_operation : public IO_operation {
public:
    Receive_operation(io_uring* ring, int fd, std::vector<uint8_t>& buffer)
        : IO_operation(ring), m_fd(fd), m_buffer(buffer) {}

protected:
    void submit_operation() override;

private:
    int m_fd;
    sockaddr_in m_client_addr;
    std::vector<uint8_t>& m_buffer;
    socklen_t m_client_addr_len = sizeof(sockaddr_in);
};

class Socket {
public:
    explicit Socket(uint16_t port);
    ~Socket();

    Task<size_t> send_async(const std::string& address, uint16_t port, const std::vector<uint8_t>& data);
    Task<size_t> receive_async(std::vector<uint8_t>& buffer);

    void close();

private:
    int m_socket_fd;
    io_uring m_ring;
    bool m_is_initialized;
};

} // namespace udp

