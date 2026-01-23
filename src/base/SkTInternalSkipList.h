/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTInternalSkipList_DEFINED
#define SkTInternalSkipList_DEFINED

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkRandom.h"

#include <functional>

#define SK_DECLARE_INTERNAL_SKIPLIST_INTERFACE(ClassName, MaxLevel)       \
    friend class SkTInternalSkipList<ClassName, MaxLevel>;                \
    ClassName* fSkipNext[MaxLevel] = {nullptr};                           \
    ClassName* fSkipPrev = nullptr;                                       \
    int fSkipLevel = 0;                                                   \
    SkDEBUGCODE(void* fSkipList = nullptr;)

template <class T, int N = 16, typename LESS = std::less<T>>
class SkTInternalSkipList {
public:
    static_assert(sizeof(T::fSkipNext) / sizeof(T*) >= N, "Node MaxLevel must be >= SkipList N");

    explicit SkTInternalSkipList(const LESS& less = LESS())
        : fLess(less) {
        this->reset();
    }

    void reset() {
        T* curr = fHead[0];
        while (curr) {
            T* next = curr->fSkipNext[0];
            for (int i = 0; i < N; ++i) curr->fSkipNext[i] = nullptr;
            curr->fSkipPrev = nullptr;
            curr->fSkipLevel = 0;
            SkDEBUGCODE(curr->fSkipList = nullptr;)
            curr = next;
        }
        for (int i = 0; i < N; ++i) fHead[i] = nullptr;
        fTail = nullptr;
    }

    /**
     * Finds the first entry in the list that matches the given key.
     * Returns nullptr if no match is found.
     */
    T* find(const T& key) const {
        T* update[N];
        // We use the search helper to find the predecessors
        this->search(key, update);

        // The candidate is the node immediately after the level-0 predecessor
        T* candidate = update[0] ? update[0]->fSkipNext[0] : fHead[0];

        // Check if candidate exists and is equal to key (not less and not greater)
        if (candidate && !fLess(key, *candidate)) {
            return candidate;
        }
        return nullptr;
    }

    /**
     * Finds the specific entry pointer in the list.
     * Useful to verify if a specific object instance is currently linked.
     */
    T* findExact(const T* entry) const {
        SkASSERT(entry);
        T* update[N];
        this->searchExact(entry, update);

        // candidate is the node after the level-0 predecessor
        T* candidate = update[0] ? update[0]->fSkipNext[0] : fHead[0];

        // In searchExact, we guarantee we stopped exactly at 'entry' if it existed.
        return (candidate == entry) ? candidate : nullptr;
    }

    /**
     * Finds the exact match for 'key'.
     * If not found, returns the node strictly immediately preceding where it would be.
     * * Returns:
     * - The Exact Node: if a node with 'key' exists.
     * - The Predecessor: if 'key' is not found, but fits after this node.
     * - nullptr: if 'key' is smaller than everything in the list (insertion at Head).
     */
    T* findLessOrEqual(const T& key) const {
        T* update[N];
        // 1. Run the standard search.
        // update[0] is guaranteed to be the largest node strictly LESS than key.
        this->search(key, update);

        T* predecessor = update[0];

        // 2. Peek at the next node to see if it is an Exact Match
        // If predecessor is null, we are at head, so check fHead[0].
        T* candidate = predecessor ? predecessor->fSkipNext[0] : fHead[0];

        // If candidate exists and is NOT less than key, it must be Equal (since predecessor was Less)
        // Note: We use !fLess(key, candidate) to check equality.
        if (candidate && !fLess(key, *candidate)) {
            return candidate; // Exact match found
        }

        // 3. No exact match, return the node strictly prior
        return predecessor;
    }

    void insert(T* entry) {
        SkASSERT(entry);
        SkASSERT(nullptr == entry->fSkipList);

        T* update[N];
        this->search(*entry, update);

        int level = this->randomLevel();
        entry->fSkipLevel = level;

        for (int i = 0; i < level; ++i) {
            if (update[i] == nullptr) {
                entry->fSkipNext[i] = fHead[i];
                fHead[i] = entry;
            } else {
                entry->fSkipNext[i] = update[i]->fSkipNext[i];
                update[i]->fSkipNext[i] = entry;
            }
        }

        entry->fSkipPrev = update[0];
        T* nextAtLevel0 = entry->fSkipNext[0];
        if (nextAtLevel0) {
            nextAtLevel0->fSkipPrev = entry;
        } else {
            fTail = entry;
        }

        SkDEBUGCODE(entry->fSkipList = this;)
    }

    void remove(T* entry) {
        SkASSERT(entry);
        SkASSERT(this->isInList(entry));

        T* update[N];
        this->searchExact(entry, update);

        for (int i = 0; i < entry->fSkipLevel; ++i) {
            if (update[i] == nullptr) {
                SkASSERT(fHead[i] == entry);
                fHead[i] = entry->fSkipNext[i];
            } else {
                SkASSERT(update[i]->fSkipNext[i] == entry);
                update[i]->fSkipNext[i] = entry->fSkipNext[i];
            }
        }

        T* nextNode = entry->fSkipNext[0];
        if (nextNode) {
            nextNode->fSkipPrev = entry->fSkipPrev;
        } else {
            fTail = entry->fSkipPrev;
        }

        for(int i=0; i < N; ++i) entry->fSkipNext[i] = nullptr;
        entry->fSkipPrev = nullptr;
        entry->fSkipLevel = 0;

        SkDEBUGCODE(entry->fSkipList = nullptr;)
    }

    bool contains(const T* entry) const {
        return this->isInList(entry);
    }

    bool isEmpty() const { return fHead[0] == nullptr; }
    T* head() const { return fHead[0]; }
    T* tail() const { return fTail; }

    class Iter {
    public:
        enum IterStart { kHead_IterStart, kTail_IterStart };
        Iter() : fCurr(nullptr) {}
        T* init(const SkTInternalSkipList& list, IterStart startLoc) {
            fCurr = (kHead_IterStart == startLoc) ? list.fHead[0] : list.fTail;
            return fCurr;
        }
        T* get() { return fCurr; }
        T* next() { if (!fCurr) return nullptr; fCurr = fCurr->fSkipNext[0]; return fCurr; }
        T* prev() { if (!fCurr) return nullptr; fCurr = fCurr->fSkipPrev; return fCurr; }
    private:
        T* fCurr;
    };

#ifdef SK_DEBUG
    bool isInList(const T* entry) const { return entry->fSkipList == this; }
    void validate() const {
        T* prev = nullptr;
        T* curr = fHead[0];
        while (curr) {
            SkASSERT(curr->fSkipPrev == prev);
            SkASSERT(this->isInList(curr));
            prev = curr;
            curr = curr->fSkipNext[0];
        }
        SkASSERT(fTail == prev);
    }
#endif

private:
    T* fHead[N] = {nullptr};
    T* fTail = nullptr;
    SkRandom fRandom;
    LESS fLess;

    int randomLevel() {
        int lvl = 1;
        while (lvl < N && (fRandom.nextU() & 0x80000000)) lvl++;
        return lvl;
    }

    /**
     * Fills 'update' with the pointers to the nodes strictly LESS than 'key'.
     * update[i] will contain the node at level 'i' that should precede 'key'.
     * Used for find() and insert().
     */
    void search(const T& key, T** update) const {
        T* curr = nullptr;
        for (int i = N - 1; i >= 0; --i) {
            T* next = (curr == nullptr) ? fHead[i] : curr->fSkipNext[i];
            while (next && fLess(*next, key)) {
                curr = next;
                next = curr->fSkipNext[i];
            }
            update[i] = curr;
        }
    }

    /**
     * Fills 'update' with pointers to the nodes immediately preceding 'entry'.
     * Requires checking pointer identity to handle duplicates correctly.
     * Used for findExact() and remove().
     */
    void searchExact(const T* entry, T** update) const {
        T* curr = nullptr;
        for (int i = N - 1; i >= 0; --i) {
            T* next = (curr == nullptr) ? fHead[i] : curr->fSkipNext[i];

            // 1. Value search: Move forward while value is strictly less
            while (next && fLess(*next, *entry)) {
                curr = next;
                next = curr->fSkipNext[i];
            }

            // 2. Exact search: If we are at a level where 'entry' exists...
            // We must traverse the duplicate chain to find the exact pointer.
            if (i < entry->fSkipLevel) {
                 while (next && next != entry) {
                    curr = next;
                    next = curr->fSkipNext[i];
                }
            }
            update[i] = curr;
        }
    }

    SkTInternalSkipList(const SkTInternalSkipList&) = delete;
    SkTInternalSkipList& operator=(const SkTInternalSkipList&) = delete;
};

#endif
