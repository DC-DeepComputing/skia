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

constexpr int kNumWorkLists = 2;
constexpr int kNumThreads = 4;
constexpr int kHighPriority = 0;
constexpr int kLowPriority = 1;

class Collector {
public:
    static constexpr int kMaxCount = 200;

    void insert(int i) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        if (fCount >= kMaxCount) {
            return;
        }

        fData[fCount++] = i;
    }

    int count() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};
        return fCount;
    }

    void dump() const SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        for (int i = 0; i < fCount; i++) {
            SkDebugf("%d ", fData[i]);
        }
        SkDebugf("\n");
    }

private:
    mutable SkSpinlock fSpinLock;

    int fCount SK_GUARDED_BY(fSpinLock) = 0;
    int fData[kMaxCount] SK_GUARDED_BY(fSpinLock);
};

// Make sure all high priority work is started before the low priority work is begun
void priority_test(skiatest::Reporter* reporter, std::unique_ptr<SkExecutor> executor) {
    Collector collector;

    for (int i = 0; i < 100; ++i) {
        executor->add2([&collector, i]() { collector.insert(i); }, kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        executor->add2([&collector, i]() { collector.insert(i); }, kLowPriority);
    }
    executor.reset();

    SkDebugf("%d\n", collector.count());
    REPORTER_ASSERT(reporter, collector.count() == Collector::kMaxCount);
    collector.dump();
}

// Test out discarding
void discard_test(skiatest::Reporter* reporter, std::unique_ptr<SkExecutor> executor) {
    Collector collector;

    for (int i = 0; i < 100; ++i) {
        executor->add2([&collector, i]() { collector.insert(i); }, kHighPriority);
    }
    for (int i = 100; i < 200; ++i) {
        executor->add2([&collector, i]() { collector.insert(i); }, kLowPriority);
    }
    executor->discardAllPendingWork();

    executor.reset();

    // We should've shaved off some work
    REPORTER_ASSERT(reporter, collector.count() < Collector::kMaxCount);
    collector.dump();
}

std::unique_ptr<SkExecutor> make_FIFO() {
    return SkExecutor::MakeMultiListFIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

std::unique_ptr<SkExecutor> make_LIFO() {
    return SkExecutor::MakeMultiListLIFOThreadPool(
        kNumWorkLists, kNumThreads, /* allowBorrowing= */ false);
}

} // anonymous namespace


DEF_TEST(Executor, reporter) {
    for (auto makeExecutor : { make_FIFO, make_LIFO }) {
        priority_test(reporter, makeExecutor());
        discard_test(reporter, makeExecutor());
    }
}
