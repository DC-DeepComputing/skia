/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkTaskGroup.h"

#include "include/core/SkExecutor.h"

#include <type_traits>
#include <utility>

template <typename Work>
SkTaskGroup<Work>::SkTaskGroup(SkExecutor<Work>& executor) : fPending(0), fExecutor(executor) {}

template <typename Work>
void SkTaskGroup<Work>::add(Work work) {
    this->add(std::move(work), /* workList= */ 0);
}

template <typename Work>
void SkTaskGroup<Work>::add(Work work, int workList) {
    fPending.fetch_add(+1, std::memory_order_relaxed);
    fExecutor.add([this, work2{std::move(work)}] {
                      work2();
                      fPending.fetch_add(-1, std::memory_order_release);
                  },
                  workList);
}

template <typename Work>
void SkTaskGroup<Work>::discardAllPendingWork() {
    int numDiscarded = fExecutor.discardAllPendingWork();
    fPending.fetch_add(-numDiscarded, std::memory_order_release);
}

#if 0
void SkTaskGroup::batch(int N, std::function<void(int)> fn) {
    // TODO: I really thought we had some sort of more clever chunking logic.
    fPending.fetch_add(+N, std::memory_order_relaxed);
    for (int i = 0; i < N; i++) {
        fExecutor.add([fn, i, this] {
            fn(i);
            fPending.fetch_add(-1, std::memory_order_release);
        });
    }
}
#endif

template <typename Work>
bool SkTaskGroup<Work>::done() const {
    return fPending.load(std::memory_order_acquire) == 0;
}

template <typename Work>
void SkTaskGroup<Work>::wait() {
    // Actively help the executor do work until our task group is done.
    // This lets SkTaskGroups nest arbitrarily deep on a single SkExecutor:
    // no thread ever blocks waiting for others to do its work.
    // (We may end up doing work that's not part of our task group.  That's fine.)
    while (!this->done()) {
        fExecutor.borrow();
    }
}

template <typename Work>
SkTaskGroup<Work>::Enabler::Enabler(int threads) {
    if (threads) {
        fThreadPool = SkExecutor<Work>::MakeLIFOThreadPool(threads);
        SkExecutor<Work>::SetDefault(fThreadPool.get());
    }
}
