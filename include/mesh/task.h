#pragma once

#include "libudp/task.h"

namespace mesh {

struct Node;

} // namespace mesh

namespace udp {

template<>
struct Task<mesh::Node*> {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {
    std::exception_ptr exception;

    Task get_return_object() noexcept {
      return Task(std::coroutine_handle<promise_type>::from_promise(*this));
    }

    std::suspend_never initial_suspend() noexcept {
      return {};
    }

    std::suspend_always final_suspend() noexcept {
      return {};
    }

    void return_void() noexcept {}

    void unhandled_exception() noexcept {
      exception = std::current_exception();
    }

    mesh::Node* m_node;
  };

  Task() noexcept : m_handle(nullptr) {}

  explicit Task(handle_type handle) noexcept : m_handle(handle) {}

  ~Task() {
    if (m_handle) {
      m_handle.destroy();
    }
  }

  Task(Task&& rhs) noexcept
    : m_handle(rhs.m_handle) {
    rhs.m_handle = nullptr;
  }

  Task& operator=(Task&& rhs) noexcept {
    if (this != &rhs) {
      if (m_handle) {
        m_handle.destroy();
      }
      m_handle = rhs.m_handle;

      rhs.m_handle = nullptr;
    }

    return *this;
  }

  bool await_ready() const noexcept;

  bool await_suspend(std::coroutine_handle<> handle);

  mesh::Node* await_resume();

  handle_type m_handle;
};

} // namespace udp
