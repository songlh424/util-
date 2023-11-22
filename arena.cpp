#include "arena.h"

static const int kBlockSize = 4096;     // 4kB

Arena::Arena()
    : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

Arena::~Arena() {
    // 没有释放内存的接口，等对象析构时同时释放所有分配的内存空间
    for(size_t i = 0; i < blocks_.size(); i++) {
        delete [] blocks_[i];
    }
}

// 当现有的内存块不满足需求时调用
char* Arena::AllocateFallback(size_t bytes) {
    if(bytes > kBlockSize / 4) {    // bytes > 1KB
    // 对象超过块的1/4，所以单独分配，避免引起分配新的内存块而丢弃的原来块还有很大的空间
        char* result = AllocateNewBlock(bytes);
        return result;
    }

    // 直接丢弃当前块的剩余空间，重新分配一个新的4K大小的块
    alloc_ptr_ = AllocateNewBlock(kBlockSize);
    alloc_bytes_remaining_ = kBlockSize;

    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
}

char* Arena::AllocateAligned(size_t bytes) {
    // 判断几字节对齐
    const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
    static_assert((align & (align - 1)) == 0, "Pointer size should be a power of 2");

    // 如果current_mod为0则满足对齐条件，如果不为0则需移动指针
    size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
    size_t slop = (current_mod == 0 ? 0 : align - current_mod);
    size_t needed = bytes + slop;           // 需要多余的slop个字节来满足对齐
    char* result;
    if(needed <= alloc_bytes_remaining_) {
        result = alloc_ptr_ + slop;
        alloc_ptr_ += needed;
        alloc_bytes_remaining_ -= needed;
    } else {
        // AllocateFallback总是返回内存对齐的块
        result = AllocateFallback(bytes);
    }
    assert((reinterpret_cast<uintptr_t>(result) & (align - 1)) == 0);
    return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
    char* result = new char[block_bytes];
    blocks_.push_back(result);
    // 加上了指针的内存消耗，vector会保存每块内存的起始地址
    memory_usage_.fetch_add(block_bytes + sizeof(char*), std::memory_order_relaxed);
    
    return result;
}