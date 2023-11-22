#include "CountDownLatch.h"
#include <iostream>
#include <thread>


int main() {
    CountDownLatch cdl(5);
    for(int i = 0; i < 5; i++) {
        std::thread t([&]() {
            std::cout << "thread: " << std::this_thread::get_id() << "created!" << '\n';
            cdl.countDown();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // 一般情况主线程执行完毕，下面语句无法执行
            std::cout << "thread: " << std::this_thread::get_id() << " main thread countDown" << '\n';
        });
        t.detach();
    }
    cdl.wait();
    std::cout << "main thread wait all subThread created" << '\n';
}