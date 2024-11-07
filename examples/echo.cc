#include <iostream>
#include <thread>

#include "libudp/socket.h"

using namespace udp;

struct Server {
  /**
   * Constructor
   *
   * @param[in] port The port to listen on
   */
  Server(uint16_t port) : m_socket(port) {
    log_info("Server listening on port " + std::to_string(port) + "...");
  }

  /**
   * Run the server

   * @param[in] client_port The port of the client to send to
   */
  void run(uint16_t client_port) noexcept;

  Socket m_socket;
};

void Server::run(uint16_t client_port) noexcept {
  try {
    Buffer buffer(32);

    for (int i = 0; i < 5; ++i) {
      auto recv_task = m_socket.receive_async(buffer);

      if (!recv_task.is_done()) {
        recv_task.resume();
      }

      auto send_task = m_socket.send_async("127.0.0.1", client_port, buffer.data(), recv_task.get_result());

      if (!send_task.is_done()) {
        send_task.resume();
      }

      log_info("Server received: ", std::string(buffer.begin(), buffer.begin() + recv_task.get_result()));

      assert(recv_task.get_result() == send_task.get_result());

      log_info("Server sent: ", send_task.get_result(), " bytes");
    }
  } catch (const std::exception& e) {
    log_error("Server error: " + std::string(e.what()));
    exit(EXIT_FAILURE);
  }
}

struct Client {
  /**
   * Constructor
   *
   * @param[in] port The port to listen on
   */
  Client(uint16_t port) : m_socket(port) {
    log_info("Client listening on port " + std::to_string(port) + "...");
  }

  /**
   * Run the client

   * @param[in] server_port The port of the server to connect to
   */
  void run(uint16_t server_port) noexcept;

  Socket m_socket;
};

void Client::run(uint16_t server_port) noexcept {
  const std::string message = "Hello, world!";

  try {
    for (int i = 0; i < 5; ++i) {
      auto send_task = m_socket.send_async("127.0.0.1", server_port, message.c_str(), message.size());

      if (!send_task.is_done()) {
        send_task.resume();
      }

      Buffer buffer(32);

      auto recv_task = m_socket.receive_async(buffer);

      if (!recv_task.is_done()) {
        recv_task.resume();
      }

      std::string echo(buffer.begin(), buffer.begin() + recv_task.get_result());

      log_info("Client received: ", echo);
    }
  } catch (const std::exception& e) {
    log_error("Client error: " + std::string(e.what()));
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {
  Logger::get_instance().set_level(Logger::Level::INFO);

  log_info("Starting UDP echo example...");

  uint16_t server_port = 8080;
  uint16_t client_port = 8081;

  if (argc == 2) {
    client_port = std::atoi(argv[1]);
  } else if (argc == 3) {
    server_port = std::atoi(argv[1]);
    client_port = std::atoi(argv[2]);
  } else if (argc > 3) {
    log_error("Usage: " + std::string(argv[0]) + " <server_port> <client_port>");
    return EXIT_FAILURE;
  }

  std::thread server_thread([server_port, client_port]() {
    Server server(server_port);

    server.run(client_port);
  });

  /* Give the server time to start */
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  Client client(client_port);

  client.run(server_port);

  /* Cleanup */
  server_thread.join();

  return 0;
}

