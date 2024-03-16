#pragma once

#include <coroutine>
#include <iostream>
#include <functional>
#include <string>

template<typename T>
class Coroutine {
public:
    struct promise_type {
        T value_;
        auto initial_suspend() noexcept { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() noexcept {}
        Coroutine get_return_object() { return Coroutine{ handle_type::from_promise(*this) }; }
        void return_void() {}
        template<std::convertible_to<T> From>
        std::suspend_always yield_value(From&& from) {
            value_ = std::forward<From>(from);
            return {};
        }
    };
    using handle_type = std::coroutine_handle<promise_type>;
    explicit Coroutine(handle_type handle) : coro_handle_(handle) { start_time_ = time(nullptr); }
    ~Coroutine() {}
    handle_type GetHandle() const {
        return coro_handle_;
    }
    time_t GetStartTime(){
        return start_time_;
    }
    time_t start_time_ = 0;
private:
    handle_type coro_handle_;
};

template<typename T>
class Awaitable {
public:
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        coro_handle_ = h;
        handler_.operator()(this);
    }
    T await_resume() { return data_; }
    T data_;
    std::function< void(Awaitable<T>* awaitable)> handler_;
    std::coroutine_handle<> coro_handle_;
};