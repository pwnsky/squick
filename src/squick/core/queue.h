

#ifndef SQUICK_QUEUE_H
#define SQUICK_QUEUE_H

#include "platform.h"
#include "third_party/concurrentqueue/concurrentqueue.h"
#include <atomic>
#include <list>
#include <mutex>
#include <thread>

class Lock {
  public:
    explicit Lock() { flag.clear(); }

    ~Lock() {}

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire))
            ;
    }

    bool try_lock() {
        if (flag.test_and_set(std::memory_order_acquire)) {
            return false;
        }

        return true;
    }

    void unlock() { flag.clear(std::memory_order_release); }

  protected:
    mutable std::atomic_flag flag = ATOMIC_FLAG_INIT;

  private:
    Lock &operator=(const Lock &src);
};

// read prior or write prior?
// it's different for these two situations

template <typename T>
// class Queue : public Lock
class Queue : public moodycamel::ConcurrentQueue<T> {
  public:
    Queue() {}

    virtual ~Queue() {}

    bool Push(const T &object) {
        this->enqueue(object);

        return true;
    }

    bool PushBulk(const T &object) {
        this->enqueue(object);

        return true;
    }

    bool TryPop(T &object) { return this->try_dequeue(object); }

    bool TryPopBulk(T &object) { return this->try_dequeue(object); }
};

#endif