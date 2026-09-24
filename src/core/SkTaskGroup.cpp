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

template <typename ExecutorType>
SkTaskGroup<ExecutorType>::SkTaskGroup(ExecutorType& executor) : fPending(0), fExecutor(executor) {}

template <typename ExecutorType>
void SkTaskGroup<ExecutorType>::add1(ExecutorType::WorkType work, int workList) {
    fPending.fetch_add(+1, std::memory_order_relaxed);
#if 0
    fExecutor.add1([this, work2{std::move(work)}] {
                      ExecutorType::WorkTraits::Execute(work2);
                      fPending.fetch_add(-1, std::memory_order_release);
                  },
                  workList);
#endif
}

template <typename ExecutorType>
bool SkTaskGroup<ExecutorType>::done1() const {
    return fPending.load(std::memory_order_acquire) == 0;
}

template <typename ExecutorType>
void SkTaskGroup<ExecutorType>::wait1() {
    // Actively help the executor do work until our task group is done.
    // This lets SkTaskGroups nest arbitrarily deep on a single SkExecutor:
    // no thread ever blocks waiting for others to do its work.
    // (We may end up doing work that's not part of our task group.  That's fine.)
    while (!this->done1()) {
        fExecutor.borrow();
    }
}

template <typename ExecutorType>
SkTaskGroup<ExecutorType>::Enabler1::Enabler1(int threads) {
    if (threads) {
        fThreadPool = ExecutorType::MakeLIFOThreadPool(threads);
        ExecutorType::SetDefault(fThreadPool.get());
    }
}

template class SkTaskGroup<StdFunctionExecutor>;
template class SkTaskGroup<CallbackExecutor>;
