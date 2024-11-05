cmake_minimum_required(VERSION 3.15)

project(libudp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LIBNET_SOURCES src/socket.cc)

SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_library(${PROJECT_NAME} STATIC ${LIBNET_SOURCES})

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  ADD_DEFINITIONS(-DUNIV_LINUX)
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

find_package(Threads REQUIRED)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE uring)

option(BUILD_TESTING "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Testing configuration
if(BUILD_TESTING)
    enable_testing()

    # Include FetchContent for downloading GTest
    include(FetchContent)

    # Download and configure Google Test
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Create test executable
    add_executable(libudp_tests tests/udp_socket_test.cc)

    # Link against our library and GTest
    target_link_libraries(net_tests
        PRIVATE
            net
            GTest::gtest_main
            GTest::gmock_main
            caspaxos
    )

    target_link_libraries(caspaxos_tests
        PRIVATE
            caspaxos
            GTest::gtest_main
            GTest::gmock_main
            caspaxos
    )

    # Include test directory
    target_include_directories(net_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(caspaxos_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # Discover tests
    include(GoogleTest)
    gtest_discover_tests(net_tests)
    gtest_discover_tests(caspaxos_tests)

    # Add custom target for running tests with detailed output
    add_custom_target(test_verbose COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS net_tests caspaxos_tests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT libudp-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# External dependencies downloaded from GitHub.
include(deps/git-submodule-init.cmake)

add_dependencies(${PROJECT_NAME} liburing)
