/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "src/base/SkSpinlock.h"
#include "tests/Test.h"

namespace {

class Collector {
public:
    void insert(int i) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        if (fCount >= kMaxCount) {
            return;
        }

        fData[fCount++] = i;
    }

    void reset() SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};
        fCount = 0;
    }

    void dump() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        for (int i = 0; i < fCount; i++) {
            SkDebugf("%d ", fData[i]);
        }
        SkDebugf("\n");
    }

private:
    static constexpr int kMaxCount = 200;

    mutable SkSpinlock fSpinLock;

    int fCount SK_GUARDED_BY(fSpinLock) = 0;
    int fData[kMaxCount] SK_GUARDED_BY(fSpinLock);
};

// Make sure all high priority work is started before the low priority work is begun
void run_test1(SkExecutor* executor, Collector* collector) {
    constexpr int kHighPriority = 0;
    constexpr int kLowPriority = 1;

    for (int i = 0; i < 100; ++i) {
        executor->add2([collector, i]() { collector->insert(i); }, kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        executor->add2([collector, i]() { collector->insert(i); }, kLowPriority);
    }
}

// Test out discarding
void run_test2(SkExecutor* executor, Collector* collector) {
    constexpr int kHighPriority = 0;
    constexpr int kLowPriority = 1;

    for (int i = 0; i < 100; ++i) {
        executor->add2([collector, i]() { collector->insert(i); }, kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        executor->add2([collector, i]() { collector->insert(i); }, kLowPriority);
    }
    executor->discardAllPendingWork();
}

} // anonymous namespace


DEF_TEST(Executor, reporter) {
    Collector collector;

    {
        std::unique_ptr<SkExecutor> executor = SkExecutor::MakeMultiQueueFIFOThreadPool(2, 4, false);
        run_test1(executor.get(), &collector);
    }

    collector.dump();

    collector.reset();

    {
        std::unique_ptr<SkExecutor> executor = SkExecutor::MakeMultiQueueFIFOThreadPool(2, 4, false);
        run_test2(executor.get(), &collector);
    }

    collector.dump();

}
