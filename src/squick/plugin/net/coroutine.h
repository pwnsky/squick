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
    explicit Coroutine(handle_type handle) : coro_handle_(handle) { started_time_ = time(nullptr); }
    ~Coroutine() {}
    handle_type GetHandle() const {
        return coro_handle_;
    }
    time_t started_time_ = 0;
private:
    handle_type coro_handle_;
};

template<typename T>
class Awaitable {
public:
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        coroutine_handle_ = h;
        handler_.operator()(req_id, this);
    }
    T* await_resume() { return data_; }
    T* data_ = nullptr;
    int req_id = 0;
    std::function< void(const int req_id, Awaitable<T>* awaitable)> handler_;
    std::coroutine_handle<> coroutine_handle_;
};