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
    for (PathOpsThreadedRunnable* runnable : fRunnables) {
        delete runnable;
    }
}

void PathOpsThreadedTestRunner::render() {
    SkStdFunctionTaskGroup taskGroup;
    for (PathOpsThreadedRunnable* runnable : fRunnables) {
        taskGroup.add1([runnable]() { (*runnable)(); });
    }
    taskGroup.wait1();
}
