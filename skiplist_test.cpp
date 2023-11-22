#include "skiplist.h"
#include "arena.h"

#include <set>

typedef uint64_t Key;

struct Comparator {
    int operator()(const Key& a, const Key& b) const {
        if(a < b) {
            return -1;
        } else if(a > b) {
            return +1;
        } else {
            return 0;
        }
    }
};

void test_empty() {
    Arena arena;
    Comparator cmp;
    SkipList<Key, Comparator> list(cmp, &arena);
    assert(!list.Contains(10));

    SkipList<Key, Comparator>::Iterator iter(&list);
    assert(!iter.Valid());

    iter.SeekToFirst();
    assert(!iter.Valid());

}

void test_InsertAndLookup() {
    const int N = 2000;
    const int R = 5000;
    std::set<Key> keys;
    Arena arena;
    Comparator cmp;
    SkipList<Key, Comparator> list(cmp, &arena);
    for(int i = 0; i < N; i++) {
        Key key = rand() % R;
        if(keys.insert(key).second) {
            list.Insert(key);
        }
    }

    for(int i = 0; i < R; i++) {
        if(list.Contains(i)) {
            assert(keys.count(i) == 1);
        } else {
            assert(keys.count(i) == 0);
        }
    }
}

int main() {
    test_empty();
    test_InsertAndLookup();
    return 0;
}