#pragma once
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

class Arena {
public:
    Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    ~Arena();

    // 分配bytes大小的内存块，并返回内存块的地址
    char* Allocate(size_t bytes);

    // 保证内存对齐的分配接口
    char* AllocateAligned(size_t bytes);

    // 返回内存使用量的估计值
    size_t MemoryUsage() const {
        memory_usage_.load(std::memory_order_relaxed);
        return memory_usage_;
    }
private:
    /// @brief 剩余空间不满足要求时调用
    char* AllocateFallback(size_t bytes);
    /// @brief 分配给较大对象时调用，直接分配其相应大小的内存块
    char* AllocateNewBlock(size_t block_bytes);

    char* alloc_ptr_;                           // 指向当前内存块
    size_t alloc_bytes_remaining_;              // 当前内存块的剩余空间
    std::vector<char*> blocks_;                 // 记录所有内存块的头指针
    std::atomic<size_t> memory_usage_;          // 统计使用的内存量
};

// 大部分就是调用这个接口来分配内存，设为内联函数
inline char* Arena::Allocate(size_t bytes) {
    // 如果允许0字节分配，语义上来讲有些混乱，所以这里直接禁止
    assert(bytes > 0);
    if(bytes <= alloc_bytes_remaining_) {
        char* result = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return result;
    }
    return AllocateFallback(bytes);
}