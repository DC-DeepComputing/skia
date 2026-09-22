/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedMutex.h"
#include "src/core/SkTaskGroup.h"
#include "tests/Test.h"

#include <functional>

DEF_TEST(SkSharedMutexBasic, r) {
    SkSharedMutex sm;
    sm.acquire();
    sm.assertHeld();
    sm.release();
    sm.acquireShared();
    sm.assertHeldShared();
    sm.releaseShared();
}

DEF_TEST(SkSharedMutexMultiThreaded, r) {
    SkSharedMutex sm;
    static const int kSharedSize = 10;
    int shared[kSharedSize];
    int value = 0;
    for (int i = 0; i < kSharedSize; ++i) {
        shared[i] = 0;
    }
    SkStdFunctionTaskGroup taskGroup;
    for (int i = 0; i < 8; ++i) {
        taskGroup.add([&]() {
            if (i % 4 != 0) {
                for (int c = 0; c < 100000; ++c) {
                    sm.acquireShared();
                    sm.assertHeldShared();
                    int v = shared[0];
                    for (int j = 1; j < kSharedSize; ++j) {
                        REPORTER_ASSERT(r, v == shared[j]);
                    }
                    sm.releaseShared();
                }
            } else {
                for (int c = 0; c < 100000; ++c) {
                    sm.acquire();
                    sm.assertHeld();
                    value += 1;
                    for (int j = 0; j < kSharedSize; ++j) {
                        shared[j] = value;
                    }
                    sm.release();
                }
            }
        });
    }
}
