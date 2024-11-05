#include <iostream>
#include <thread>

#include "libnet/udp_socket.h"

using namespace udp_library;

Task<void> run_echo_server() {
    try {
        Socket server(8080);
        std::cout << "Echo server listening on port 8080...\n";
        
        std::vector<uint8_t> buffer(1024);

        for (;;) {
            // Receive data
            auto bytes_received = co_await server.receive_async(buffer);
            buffer.resize(bytes_received);
            
            std::string message(buffer.begin(), buffer.end());
            std::cout << "Received: " << message << std::endl;
            
            // Echo back
            co_await server.send_async("127.0.0.1", 8081, buffer);
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

Task<void> run_echo_client() {
    try {
        Socket client(8081);
        
        for (;;) {
            std::cout << "Enter message (or 'quit' to exit): ";
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "quit") break;
            
            std::vector<uint8_t> data(input.begin(), input.end());
            
            // Send data
            co_await client.send_async("127.0.0.1", 8080, data);
            
            // Receive echo
            std::vector<uint8_t> buffer(1024);
            auto bytes_received = co_await client.receive_async(buffer);
            buffer.resize(bytes_received);
            
            std::string echo(buffer.begin(), buffer.end());
            std::cout << "Server echoed: " << echo << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Starting UDP echo example...\n";
    
    // Start server in a separate thread
    std::thread server_thread([]() {
        run_echo_server();
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Run client in main thread
    run_echo_client();
    
    // Cleanup
    server_thread.join();
    
    return 0;
}

