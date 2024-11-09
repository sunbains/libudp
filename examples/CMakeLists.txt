cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(EXAMPLES)

set(LIBS libudp pthread m uring)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(echo echo.cc)
add_executable(mesh mesh.cc)
add_executable(cli_basic cli_basic.cc)
add_executable(cli_advanced cli_advanced.cc)

target_link_libraries(echo PRIVATE ${LIBS})
target_link_libraries(mesh PRIVATE ${LIBS})

# CLI examples
target_link_libraries(cli_basic PRIVATE libudp)
target_link_libraries(cli_advanced PRIVATE libudp)
