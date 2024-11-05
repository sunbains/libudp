cmake_minimum_required(VERSION 3.15)

project(libnet_examples VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the main library
add_subdirectory(../libnet ${CMAKE_BINARY_DIR}/libnet)

# Echo example
add_executable(udp_echo udp_echo_server.cc)

target_link_libraries(udp_echo PRIVATE libnet pthread)
