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

template <typename ExecutorType>
class SkTaskGroup : SkNoncopyable {
public:
    // Tasks added to this SkTaskGroup will run on its executor.
    explicit SkTaskGroup(ExecutorType& executor = ExecutorType::GetDefault());
    ~SkTaskGroup() { this->wait1(); }

    // Add a task to this SkTaskGroup.
    void add1(ExecutorType::WorkType work, int workList = 0);

    // Returns true if all Tasks previously add()ed to this SkTaskGroup have run.
    // It is safe to reuse this SkTaskGroup once done().
    bool done1() const;

    // Block until done().
    void wait1();

    // A convenience for testing tools.
    // Creates and owns a thread pool, and passes it to SkExecutor::SetDefault().
    struct Enabler1 {
        explicit Enabler1(int threads = -1);  // -1 -> num_cores, 0 -> noop
        std::unique_ptr<ExecutorType> fThreadPool;
    };

private:
    std::atomic<int32_t> fPending;
    ExecutorType&        fExecutor;
};

typedef SkTaskGroup<StdFunctionExecutor> SkStdFunctionTaskGroup;
typedef SkTaskGroup<CallbackExecutor> SkCallbackTaskGroup;

#endif//SkTaskGroup_DEFINED
