#pragma once

#include <condition_variable>
#include <mutex>

#include "noncopyable.h"

/// @brief 线程间同步方法
//         一般用于某一线程等待count数量的其他线程执行到某一步去调用countDown()方法
//         当所有其他线程都执行了countDown()方法后，该线程才能执行后续操作
class CountDownLatch : noncopyable {
public:
    using MutexLock = std::mutex;
    using MutexLockGuard = std::unique_lock<MutexLock>;
    using Condition = std::condition_variable;

    explicit CountDownLatch(int count);

    void wait();
    void countDown();
    int getCount() const;

private:
    mutable MutexLock mutex_;
    Condition condition_;
    int count_;
};