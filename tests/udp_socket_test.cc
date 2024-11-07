#include <gtest/gtest.h>
#include <thread>

#include "libudp/socket.h"

class Socket_test : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(Socket_test, CreateSocket) {
    EXPECT_NO_THROW({ udp::Socket socket(12345); });
}

TEST_F(Socket_test, SendReceive) {
    Logger::get_instance().set_level(Logger::Level::WARN);

    udp::Socket client(12345);
    udp::Socket server(12346);

    udp::Buffer send_data = {'t', 'e', 's', 't'};
    udp::Buffer receive_buffer(send_data.size());

    auto server_receive_task = server.receive_async(receive_buffer);
    auto client_send_task = client.send_async("127.0.0.1", 12346, send_data.data(), send_data.size());

    server.wait_for_completion();
    client.wait_for_completion();

    std::string message(receive_buffer.begin(), receive_buffer.end());

    auto server_send_task = server.send_async("127.0.0.1", 12345, message.data(), message.size());
    auto client_receive_task = client.receive_async(receive_buffer);

    server.wait_for_completion();
    client.wait_for_completion();

    auto sent_bytes = client_send_task.get_result();
    auto received_bytes = client_receive_task.get_result();

    EXPECT_EQ(sent_bytes, send_data.size());
    EXPECT_EQ(received_bytes, send_data.size());
    EXPECT_EQ(receive_buffer[0], send_data[0]);
}
