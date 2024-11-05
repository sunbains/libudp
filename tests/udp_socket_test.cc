#include <gtest/gtest.h>
#include <thread>

#include "liudp/socket.h"

class Socket_test : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(Socket_test, CreateSocket) {
    EXPECT_NO_THROW({ udp::Socket socket(12345); });
}

TEST_F(Socket_test, SendReceive) {
    udp::Socket sender(12345);
    udp::Socket receiver(12346);

    std::vector<uint8_t> send_data = {'t', 'e', 's', 't'};
    std::vector<uint8_t> receive_buffer(1024);

    auto receive_task = receiver.receive_async(receive_buffer);
    auto send_task = sender.send_async("127.0.0.1", 12346, send_data);

    auto sent_bytes = send_task.get_result();
    auto received_bytes = receive_task.get_result();

    EXPECT_EQ(sent_bytes, send_data.size());
    EXPECT_EQ(received_bytes, send_data.size());
    EXPECT_EQ(receive_buffer[0], send_data[0]);
}
