#pragma once

#include <any>
#include <coroutine>
#include <stdexcept>
#include <type_traits>

#include "logger.h"

struct io_uring;
struct io_uring_cqe;

namespace udp {

template<typename T> struct Task {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type {

    Task get_return_object() noexcept {
      return Task(handle_type::from_promise(*this));
    }

    std::suspend_never initial_suspend() noexcept {
      log_debug("Initial suspending task: ", this);
      return {};
    }

    std::suspend_always final_suspend() noexcept {
      log_debug("Final suspending task: ", this);
      return {};
    }

    void unhandled_exception() noexcept {
      m_exception = std::current_exception();
      // TODO: Handle unhandled exceptions
      std::terminate();
    }

    template<typename U>
    void return_value(U&& value) noexcept {
      m_result = std::forward<U>(value);
    }

    T m_result;
    std::exception_ptr m_exception;
  };

  Task() noexcept : m_handle(nullptr) {}

  explicit Task(handle_type handle) noexcept : m_handle(handle) {}

  ~Task() {
    log_debug("Destroying task: ", this);

    if (m_handle) {
      m_handle.destroy();
    }
  }

  Task(Task&& rhs) noexcept : m_handle(rhs.m_handle) {
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

  void resume() {
    if (m_handle && !m_handle.done()) {
      m_handle.resume();
    }
  }

  bool is_done() const noexcept {
    return m_handle.done();
  }

  T get_result() {
    if (m_handle.promise().m_exception) {
      std::rethrow_exception(m_handle.promise().m_exception);
    } else {
      return std::move(m_handle.promise().m_result);
    }
  }

  handle_type m_handle{};
};

template<>
struct Task<std::any> {
  struct promise_type {
    std::exception_ptr exception;

    Task get_return_object() noexcept {
      return Task(std::coroutine_handle<promise_type>::from_promise(*this));
    }

    std::suspend_never initial_suspend() noexcept { return {}; }

    std::suspend_never final_suspend() noexcept { return {}; }

    void return_void() noexcept {}

    void unhandled_exception() noexcept {
      exception = std::current_exception();
    }
  };

  using handle_type = std::coroutine_handle<promise_type>;

  Task() noexcept : m_handle(nullptr) {}

  explicit Task(handle_type handle) noexcept : m_handle(handle) {}

  ~Task() {
    if (m_handle) {
      m_handle.destroy();
    }
  }

  Task(Task&& rhs) noexcept : m_handle(rhs.m_handle) {
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

  void get_result() {
    if (m_handle.promise().exception) {
      std::rethrow_exception(m_handle.promise().exception);
    }
  }

  handle_type m_handle;
};

 /** Awaitable wrapper for io_uring operations */
 struct IO_operation {

  enum class Type {
    NONE,
    SEND,
    RECEIVE,
    COMPLETION
  };

  using any_handle = std::coroutine_handle<>;

  IO_operation(io_uring* ring, Type type) noexcept : m_ring(ring), m_type(type) {}

  virtual ~IO_operation() = default;

  bool await_ready() const noexcept {
    return false;
  }

  bool await_suspend(any_handle handle);

  int await_resume() {
    log_debug(type(), " operation resumed, result: ", m_result);
    return m_result;
  }

  const char* type() const noexcept {
    switch (m_type) {
      case Type::SEND:
        return "SEND";
      case Type::RECEIVE:
        return "RECEIVE";
      case Type::COMPLETION:
        return "COMPLETION";
      default:
        return "NONE";
    }
  }

  int reap();

  virtual void submit() = 0;
  virtual int reap(io_uring_cqe* cqe) = 0;

  void complete(int result) noexcept {
    m_result = result;
    m_completed = true;
    m_handle.resume();
  }

  int m_result{};
  bool m_completed{};
  io_uring* m_ring{};
  any_handle m_handle{};
  Type m_type{Type::NONE};
};

} // namespace udp
