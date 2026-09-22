/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTaskGroup.h"
#include "tests/PathOpsThreadedCommon.h"

#include <functional>

PathOpsThreadedTestRunner::~PathOpsThreadedTestRunner() {
    for (int index = 0; index < fRunnables.size(); index++) {
        delete fRunnables[index];
    }
}

void PathOpsThreadedTestRunner::render() {
#if 0
    SkStdFunctionTaskGroup().batch(fRunnables.size(), [&](int i) {
        (*fRunnables[i])();
    });
#else
    SkStdFunctionTaskGroup taskGroup;
    for (int index = 0; index < fRunnables.size(); index++) {
        taskGroup.add([runnable = fRunnables[index]]() { (*runnable)(); });
    }
#endif
}
