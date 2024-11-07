# P2P Mesh Network Library

A modern C++23 peer-to-peer mesh networking library built on UDP and io_uring, featuring coroutines for asynchronous operations.

## Features (TODO)

- Modern C++20 implementation
- Asynchronous I/O using io_uring
- Coroutine support for clean async code
- UDP-based communication
- Automatic peer discovery
- Message routing with TTL
- Dead peer detection
- Example applications included

## Requirements

- C++23 compatible compiler
- CMake 3.15 or higher
- liburing development files
- Linux operating system (due to io_uring dependency)

### Ubuntu/Debian
```bash
sudo apt install cmake build-essential liburing-dev
```

### Fedora/RHEL
```bash
sudo dnf install cmake gcc-c++ liburing-devel
```

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/mesh-network.git
cd mesh-network

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests (optional)
ctest

# Install (optional)
sudo cmake --install .
```

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── mesh/                # P2P Mesh library
├── include/             # Core UDP networking library
│   ├── libudp/
│   │   └── ...
│   └── CMakeLists.txt
├── examples/            # Example applications
│   ├── ...
│   └── CMakeLists.txt
├── deps/                # External dependencies like liburing etc.
│   └── install/
├── src/                 # Core UDP networking library
│   └── ...
└── tests/               # Unit tests
    ├── ...
    └── CMakeLists.txt
```

## Usage Examples

### Basic UDP Communication

```cpp
#include <libudp/socket.h>

// Create a UDP socket
Socket socket(8080);

// Send data
std::vector data = {1, 2, 3, 4};
co_await socket.send_async("127.0.0.1", 8081, data);

// Receive data
std::vector buffer(1024);
auto bytes_received = co_await socket.receive_async(buffer);
```

### P2P Mesh Network

```cpp
#include <mesh/node.hpp>

// Create a mesh node
Node node(8080, "node1");

// Set up message handler
node.on_message([](const Message& msg) {
    std::cout << "Received: " << msg.payload.size() << " bytes\n";
});

// Start the node
co_await node.start();

// Connect to a peer
node.add_bootstrap_peer("127.0.0.1", 8081);

// Broadcast a message
std::vector data = {'H', 'e', 'l', 'l', 'o'};
co_await node.broadcast(data);
```

## Running Examples

### UDP Echo Server/Client
```bash
# Terminal 1
./examples/echo.cc

# Type messages to see them echoed back
```

### P2P Chat
```bash
# Terminal 1 - Start first node
./examples/p2p_chat 8001 alice

# Terminal 2 - Start second node
./examples/p2p_chat 8002 bob
> /connect 127.0.0.1 8001

# Available commands:
# /connect   - Connect to a peer
# /peers - List connected peers
# /quit - Exit application
# Just type to send a message
```

## Library Integration

### CMake Integration
```cmake
find_package(libudp REQUIRED)

target_link_libraries(your_target
    PRIVATE
        libudp_network_project::udp_library
        libudp_network_project::mesh
)
```

### Manual Integration
```cpp
#include <ulibudp/socket.h>
#include <mesh/mesh.h>
```

## Configuration

The library can be configured through CMake options:

```bash
cmake .. \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_TESTS=ON
```

## Performance Considerations

- TODO
- Uses io_uring for efficient async I/O
- Minimal memory allocations in critical paths
- Built-in message deduplication
- Configurable TTL for message propagation
- Automatic peer cleanup for dead connections

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Linux io_uring developers

## Troubleshooting

### Common Issues

1. Compilation Errors
```bash
# Make sure you have a C++20 compatible compiler
g++ --version
cmake --version
```

2. io_uring Issues
```bash
# Check if io_uring is available
ls /usr/include/liburing*
```

3. Port Already in Use
```bash
# Check if ports are available
sudo netstat -tulpn | grep 
```

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Support

For issues, questions, or contributions, please create an issue on the GitHub repository.

