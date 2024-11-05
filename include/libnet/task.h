#pragma once

#include <coroutine>
#include <exception>
#include <type_traits>

#include "liburing.h"

namespace udp{

template<typename T = void>
class Task {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T result;
        std::exception_ptr exception;

        Task get_return_object() noexcept {
            return Task(handle_type::from_promise(*this));
        }

        std::suspend_never initial_suspend() noexcept { return {}; }

        std::suspend_never final_suspend() noexcept { return {}; }

        void unhandled_exception() noexcept {
            exception = std::current_exception();
        }

        template<typename U>
        void return_value(U&& value) noexcept {
            result = std::forward<U>(value);
        }
    };

    Task() noexcept : m_handle(nullptr) {}

    explicit Task(handle_type handle) noexcept : m_handle(handle) {}

    ~Task() {
        if (m_handle) m_handle.destroy();
    }

    Task(Task&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_handle) m_handle.destroy();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    T get_result() {
        if (m_handle.promise().exception)
            std::rethrow_exception(m_handle.promise().exception);
        return std::move(m_handle.promise().result);
    }

private:
    handle_type m_handle;
};

template<>
class Task<void> {
public:
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
        if (m_handle) m_handle.destroy();
    }

    Task(Task&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (m_handle) m_handle.destroy();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    void get_result() {
        if (m_handle.promise().exception)
            std::rethrow_exception(m_handle.promise().exception);
    }

private:
    handle_type m_handle;
};

// Awaitable wrapper for io_uring operations
class IO_operation {
public:
    IO_operation(io_uring* ring) : m_ring(ring), m_completed(false), m_result(0) {}
    virtual ~IO_operation() = default;

    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        m_handle = handle;
        submit_operation();
    }

    int await_resume() {
        if (m_result < 0) {
            throw std::runtime_error("IO operation failed");
        }
        return m_result;
    }

protected:
    virtual void submit_operation() = 0;
    void complete(int result) {
        m_result = result;
        m_completed = true;
        m_handle.resume();
    }

    int m_result;
    bool m_completed;
    io_uring* m_ring;
    std::coroutine_handle<> m_handle;
};

