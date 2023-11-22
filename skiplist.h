#pragma once
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <fstream>
// #include <random>

#include "arena.h"

template <typename Key, class Comparator>
class SkipList {
private:
    struct Node;
public:
    explicit SkipList(Comparator cmp, Arena* arena);

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    /// @brief 向跳表插入key，不能插入重复key
    void Insert(const Key& key);
    /// @brief 判断跳表中是否包含key
    bool Contains(const Key& key) const;
    /// @brief 删除指定key的节点
    void Delete(const Key& key);
    /// @brief 跳表中的节点个数
    int Size() { return element_count_; }

    // 内部类，跳表的迭代器
    class Iterator {
    public:
        // 用一个具体的跳表来初始化迭代器
        explicit Iterator(const SkipList* list);
        // 如果迭代器的位置在一个合法的节点返回true
        bool Valid() const;
        // return 当前位置的 key，要求Valid()
        const Key& key() const;
        // 前进到下一个位置，要求Valid()
        void Next();
        // 前进到前一个位置，要求Valid()
        void Prev();
        // 前进到第一个key >= target的位置
        void Seek(const Key& target);
        // 移动到开头
        void SeekToFirst();
        // 移动到结尾
        void SeekToLast();
    private:
        const SkipList* list_;      // 迭代器作用的跳表对象
        Node* node_;                // 迭代器当前位置
    };

private:
    enum { KMaxHeight = 12 };       // 跳表的最大高度

    inline int GetMaxHeight() const {
        return max_height_.load(std::memory_order_relaxed);
    }

    /// @brief 分配一个新节点
    /// @param key 节点值
    /// @param height 节点高度
    Node* NewNode(const Key& key, int height);

    /// @brief 获得随机高度，为满足跳表有log(n)的时间复杂度，高度+1的概率保持1/4
    int RandomHeight();
    /// @brief 判断是否相等
    bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

    /// @brief 判断key是否在node节点的后面，即key是否大于node中节点
    bool KeyIsAfterNode(const Key& key, Node* n) const;
    /// @brief 找到第一个大于等于key的所有层高的节点保存在prev中
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
    /// @brief 找到最后一个小于key的节点
    Node* FindLessThan(const Key& key) const;
    /// @brief 遍历返回最后一个节点
    Node* FindLast() const;


    // 构造后就不能再变
    Comparator const compare_;          // 比较函数对象，外部传入。
                                        //比较大于返回+1，等于返回0，小于返回-1
    Node* const head_;                  // 跳表头节点
    int max_height_;       // 记录跳表的最高高度
    int element_count_;                  // 记录跳表中节点数量
};


// skiplist中的节点实现，内部成员主要是key和一个指针数组
template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
    explicit Node(const Key& k) : key(k) {}

    Key const key;

    // 取该节点第n层的指针
    Node* Next(int n) {
        assert(n >= 0);
        // 保证看到的是完整初始化版本的Node
        return next_[n];
    }

    // 设置该节点第n层的指针为x
    void SetNext(int n, Node* x) {
        assert(n >= 0);
        // 保证任何通过该指针读的人能看到完整初始化的node
        next_[n] = x;
    }
private:
    Node* next_[1];    // 利用变长结构体，在分配时才确定指针数组长度
};

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(
        const Key& key, int height) {
    
    // 根据节点的实际高度来分配对应的内存
    // char* const node_memory = arena_->AllocateAligned(
    //         sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
    // 不使用leveldb的内存管理器，使用malloc简易实现，但需在跳表析构时将所有空间释放掉
    const char* node_memory = (char*)malloc(sizeof(Node) + sizeof(Node*) * (height - 1));
    // 直接在分配的地址空间上构造，构造函数主要就是给key设值
    return new (node_memory) Node(key);
}

template <typename Key, class Comparator>
inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
    list_ = list;
    node_ = nullptr;
}

template <typename Key, class Comparator>
inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
    return node_ != nullptr;
}

template <typename Key, class Comparator>
inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
    assert(Valid());
    return node_->key;
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Next() {
    assert(Valid());
    node_ = node_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::Prev() {
    // 实现上不是通过前向指针，而是寻找最后一个不大于当前key的位置
    assert(Valid());
    node_ = list_->FindLessThan(node_->key);
    if(node_ == list_->head_) {
        node_ = nullptr;
    }
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
    node_ = list_->head_->Next(0);
}

template <typename Key, class Comparator>
inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
    node_ = list_->FindLast();
    if(node_ == list_->head_) {
        node_ = nullptr;
    }
}

template <typename Key, class Comparator>
int SkipList<Key, Comparator>::RandomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    srand((unsigned)time(NULL));    // 产生一个以当前时间开始的种子
    // 有1/4的概率为true，使得高度+1
    while(height < KMaxHeight && rand() % kBranching == 0) {
        height++;
    }
    assert(height > 0);
    assert(height <= KMaxHeight);
    return height;
}

template<typename Key, class Comparator>
bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
    // null node is considered infinite
    return (n != nullptr) && (compare_(n->key, key) < 0);
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key,
                                              Node** prev) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while(true) {
        Node* next = x->Next(level);    // 从最高层开始找
        if(KeyIsAfterNode(key, next)) {
            x = next;
        } else {
            if(prev != nullptr) prev[level] = x;
            if(level == 0) {
                return next;
            } else {
                level--;
            }
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node*
SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
    Node* x = head_;
    int level = GetMaxHeight() - 1;
    while(true) {
        assert(x == head_ || compare_(x->key, key) < 0);
        Node* next = x->Next(level);
        // 下一节点为空，或下一节点的key要大于目标key，需要下一层
        if(next == nullptr || compare_(next->key, key) >= 0) {
            if(level == 0) {
                return x;
            } else {
                level--;
            }
        } else {
            x = next;
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
    Node* x = head_;
    int level = GetMaxHeight - 1;
    while(true) {
        Node* next = x->Next(level);
        if(next == nullptr) {
            if(level == 0) {
                return x;
            } else {
                level--;
            }
        } else {
            x = next;
        }
    }
}

template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
    : compare_(cmp),
      head_(NewNode(0, KMaxHeight)),
      max_height_(1),
      element_count_(0) {
    for(int i = 0; i < KMaxHeight; i++) {
        head_->SetNext(i, nullptr);
    }
}


template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Insert(const Key& key) {
    Node* prev[KMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);

    assert(x == nullptr || !Equal(key, x->key));

    int height = RandomHeight();
    if(height > GetMaxHeight()) {
        for(int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
        }
        max_height_ = height;
    }

    x = NewNode(key, height);
    for(int i = 0; i < height; i++) {
        x->SetNext(i, prev[i]->Next(i));
        prev[i]->SetNext(i, x);
    }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Contains(const Key& key) const {
    Node* x = FindGreaterOrEqual(key, nullptr);
    if(x != nullptr && Equal(key, x->key)) {
        return true;
    } else {
        return false;
    }
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Delete(const Key& key) {
    Node* prev[KMaxHeight];
    Node* x = FindGreaterOrEqual(key, prev);
    if(x == nullptr || !Equal(key, x->key)) {
        return;
    }
    // 已找到要删除的节点x，需要

}
