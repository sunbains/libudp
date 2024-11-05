cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(EXAMPLES)

set(LIBS libudp pthread m uring)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(echo echo.cc)

target_link_libraries(echo PRIVATE ${LIBS})
