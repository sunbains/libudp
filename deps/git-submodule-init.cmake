include(FetchContent)
include(ExternalProject)

find_package(Git QUIET)

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
set(DEPS_INSTALL_DIR "${DEPS_DIR}/install")

message(STATUS "Add external dependency liburing")

set(LIBURING_SRC_DIR "${DEPS_DIR}/liburing")
set(LIBURING_INSTALL_DIR "${DEPS_INSTALL_DIR}/liburing")

set(LIBURING_LIB_DIR "${LIBURING_INSTALL_DIR}/lib")
set(LIBURING_INCLUDE_DIR "${LIBURING_INSTALL_DIR}/include")

ExternalProject_Add(
  liburing
  GIT_REPOSITORY https://github.com/axboe/liburing.git
  SOURCE_DIR ${LIBURING_SRC_DIR}
  PREFIX ${DEPS_INSTALL_DIR}
  BUILD_COMMAND make -C ${LIBURING_SRC_DIR} prefix=${LIBURING_INSTALL_DIR}
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND make
    -C ${LIBURING_SRC_DIR}
    prefix=${LIBURING_INSTALL_DIR}
    includedir=${LIBURING_INCLUDE_DIR}
    libdir=${LIBURING_LIB_DIR}
    libdevdir=${LIBURING_LIB_DIR}
    mandir=${LIBURING_INSTALL_DIR}/man
    install)

target_link_libraries(${PROJECT_NAME} PRIVATE uring)
target_include_directories(${PROJECT_NAME} PUBLIC "${LIBURING_INCLUDE_DIR}")
target_link_directories(${PROJECT_NAME} PUBLIC "${LIBURING_LIB_DIR}")

if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")

  # Update submodules as needed
 option(GIT_SUBMODULE "-- Check submodules during build" ON)

 if(GIT_SUBMODULE)

   message(STATUS "Submodule update")

   execute_process(
     COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
     WORKING_DIRECTORY ${DEPS_DIR}
     RESULT_VARIABLE GIT_SUBMOD_RESULT)

   IF(NOT ${GIT_SUBMOD_RESULT} EQUAL "0")
     message(FATAL_ERROR
             "git submodule update --init --recursive failed with "
             "${GIT_SUBMOD_RESULT}, please checkout submodules")
   endif()
 endif()
endif()
