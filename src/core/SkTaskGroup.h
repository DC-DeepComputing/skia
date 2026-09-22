/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTaskGroup_DEFINED
#define SkTaskGroup_DEFINED

#include "include/core/SkExecutor.h"
#include "include/core/SkTypes.h"
#include "include/private/SkNoncopyable.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

template <typename Work, typename WorkTraits>
class SkTaskGroup : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on its executor.
    explicit SkTaskGroup(SkExecutor<Work>& executor = SkExecutor<Work>::GetDefault());
    ~SkTaskGroup() { this->wait(); }

    // Add a task to this SkTaskGroup.
    void add(Work work);
    void add(Work work, int workList);

    void discardAllPendingWork();

#if 0
    // Add a batch of N tasks, all calling fn with different arguments.
    void batch(int N, std::function<void(int)> fn);
#endif

    // Returns true if all Tasks previously add()ed to this SkTaskGroup have run.
    // It is safe to reuse this SkTaskGroup once done().
    bool done() const;

    // Block until done().
    void wait();

    // A convenience for testing tools.
    // Creates and owns a thread pool, and passes it to SkExecutor::SetDefault().
    struct Enabler {
        explicit Enabler(int threads = -1);  // -1 -> num_cores, 0 -> noop
        std::unique_ptr<SkExecutor<Work>> fThreadPool;
    };

private:
    std::atomic<int32_t> fPending;
    SkExecutor<Work>&    fExecutor;
};

#endif//SkTaskGroup_DEFINED
