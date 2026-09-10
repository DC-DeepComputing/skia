/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkArenaAllocList_DEFINED
#define SkArenaAllocList_DEFINED

#include "include/private/SkAssert.h"
#include "src/core/SkArenaAlloc.h" // IWYU pragma: keep
#include "src/partition_alloc/raw_ptr_exclusion.h"

#include <utility>
#include "src/partition_alloc/raw_ptr.h"

/**
 * A singly linked list of Ts stored in a SkArenaAlloc. The arena rather than the list owns
 * the elements. This supports forward iteration and range based for loops.
 */
template <typename T>
class SkArenaAllocList {
private:
    struct Node;

public:
    SkArenaAllocList() = default;

    void reset() { fHead = fTail = nullptr; }

    template <typename... Args>
    inline T& append(SkArenaAlloc* arena, Args... args);

    class Iter {
    public:
        Iter() = default;
        inline Iter& operator++();
        T& operator*() const { return fCurr->fT; }
        T* operator->() const { return &fCurr->fT; }
        bool operator==(const Iter& that) const { return fCurr == that.fCurr; }
        bool operator!=(const Iter& that) const { return !(*this == that); }

    private:
        friend class SkArenaAllocList;
        explicit Iter(Node* node) : fCurr(node) {}
        // RAW_PTR_EXCLUSION: Inner-loop arena linked-list traversal cursor.
        raw_ptr<Node> fCurr = nullptr;
    };

    Iter begin() { return Iter(fHead); }
    Iter end() { return Iter(); }
    Iter tail() { return Iter(fTail); }

private:
    struct Node {
        template <typename... Args>
        Node(Args... args) : fT(std::forward<Args>(args)...) {}
        T fT;
        // RAW_PTR_EXCLUSION: Inner-loop arena linked-list node chaining.
        raw_ptr<Node> fNext = nullptr;
    };
    // RAW_PTR_EXCLUSION: Inner-loop arena linked-list head/tail pointers.
    raw_ptr<Node, DanglingUntriaged> fHead = nullptr;
    raw_ptr<Node, DanglingUntriaged> fTail = nullptr;
};

template <typename T>
template <typename... Args>
T& SkArenaAllocList<T>::append(SkArenaAlloc* arena, Args... args) {
    SkASSERT(!fHead == !fTail);
    auto* n = arena->make<Node>(std::forward<Args>(args)...);
    if (!fTail) {
        fHead = fTail = n;
    } else {
        fTail = fTail->fNext = n;
    }
    return fTail->fT;
}

template <typename T>
typename SkArenaAllocList<T>::Iter& SkArenaAllocList<T>::Iter::operator++() {
    fCurr = fCurr->fNext;
    return *this;
}

#endif
