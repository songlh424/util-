#include "arena.h"

#include <iostream>
#include <vector>

void test_empty() {
    Arena arena;
}

void test_simply() {
    std::vector<std::pair<size_t, char*>> allocated;
    Arena arena;
    const int N = 100;
    size_t bytes = 0;

    for(int i = 0; i < N; i++) {
        size_t s;
        if(i % (N / 10) == 0) {
            s = i;
        } else {
            s = rand() % 4000 + 1;
        }
        if(s == 0) {
            // 无法分配0
            s = 1;
        }
        char* r;
        if(rand() % 10) {
            r = arena.AllocateAligned(s);
        } else {
            r = arena.Allocate(s);
        }

        for(size_t b = 0; b < s; b++) {
            r[b] = i % 256;
        }
        bytes += s;
        allocated.push_back(std::make_pair(s, r));

        std::cout << "actually applied for: " << bytes << " bytes" << std::endl;
        std::cout << "memory manager has applied for: " << arena.MemoryUsage() << " bytes" << std::endl;
        // assert(arena.MemoryUsage() == bytes);
        if(i > N / 10) {
            // assert(arena.MemoryUsage() == bytes * 1.10);
        }
    }

    for(size_t i = 0; i < allocated.size(); i++) {
        size_t num_bytes = allocated[i].first;
        const char* p = allocated[i].second;
        for(size_t b = 0; b < num_bytes; b++) {
            assert(int(p[b]) & 0xff == i % 256);
        }
    }
}

int main() {
    test_empty();
    test_simply();

    return 0;
}