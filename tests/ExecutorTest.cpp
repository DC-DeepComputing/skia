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

class Foo {
public:
    void insert(int i) SK_EXCLUDES(fSpinLock) {
        SkAutoSpinlock lock{fSpinLock};

        fInts[fCount++] = i;
    }

private:
    SkSpinlock fSpinLock;

    int fCount SK_GUARDED_BY(fSpinLock) = 0;
    int fInts[200] SK_GUARDED_BY(fSpinLock);
};

void run_test(SkExecutor* executor, Foo* foo, int numWorkQueues) {

    for (int i = 0; i < 100; ++i) {
        executor->add2([foo, i]() { foo->insert(i); }, 0);
    }
    for (int i = 100; i < 200; ++i) {
        executor->add2([foo, i]() { foo->insert(i); }, 1);
    }

}

} // anonymous namespace


DEF_TEST(Executor, reporter) {

    std::unique_ptr<SkExecutor> executor = SkExecutor::MakeMultiQueueFIFOThreadPool(2, 4, false);

    Foo foo;

    run_test(executor.get(), &foo, 2);
}
