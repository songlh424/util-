#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : count_(count) {

}

void CountDownLatch::wait() {
    MutexLockGuard lock(mutex_);
    while(count_ > 0) {
        condition_.wait(lock);
    }
}

void CountDownLatch::countDown() {
    MutexLockGuard lock(mutex_);
    --count_;
    if(count_ == 0) {
        condition_.notify_all();
    }
}

int CountDownLatch::getCount() const {
    MutexLockGuard lock(mutex_);
    return count_;
}