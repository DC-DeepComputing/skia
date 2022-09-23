/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFinishCallbacks.h"
#include "src/gpu/ganesh/GrGpu.h"

GrFinishCallbacks::GrFinishCallbacks(GrGpu* gpu) : fGpu(gpu) {}

GrFinishCallbacks::~GrFinishCallbacks() {
    SkDebugf("finished callback dtor start\n");
    this->callAll(true);
    SkDebugf("finished callback dtor end\n");
}

void GrFinishCallbacks::add(GrGpuFinishedProc finishedProc,
                            GrGpuFinishedContext finishedContext) {
    SkDebugf("adding finish callback. Proc: %p, context: %p\n", finishedProc, finishedContext);
    SkASSERT(finishedProc);
    FinishCallback callback;
    callback.fCallback = finishedProc;
    callback.fContext = finishedContext;
    callback.fFence = fGpu->insertFence();
    fCallbacks.push_back(callback);
}

void GrFinishCallbacks::check() {
    SkDebugf("finish callback check 1. Num callbacks: %d\n", (int)fCallbacks.size());
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fCallbacks.empty() && fGpu->waitFence(fCallbacks.front().fFence)) {
        SkDebugf("finish callback check 2\n");
        auto finishCallback = fCallbacks.front();
        fGpu->deleteFence(finishCallback.fFence);
        SkDebugf("finish callback check 3\n");
        fCallbacks.pop_front();
        SkDebugf("finish callback check 4\n");
        SkDebugf("about to call finish callback. Proc: %p, context: %p\n", finishCallback.fCallback, finishCallback.fContext);
        finishCallback.fCallback(finishCallback.fContext);
        SkDebugf("finish callback check 5\n");
    }
    SkDebugf("finish callback check end\n");
}

void GrFinishCallbacks::callAll(bool doDelete) {
    SkDebugf("finish callback all 1\n");
    while (!fCallbacks.empty()) {
        SkDebugf("finish callback all 2\n");
        // While we are processing a proc we need to make sure to remove it from
        // the callback list before calling it. This is because the client could
        // trigger a call (e.g. calling flushAndSubmit(/*sync=*/true)) that has
        // us process the finished callbacks. We also must process deleting the
        // fence before a client may abandon the context.
        auto finishCallback = fCallbacks.front();
        if (doDelete) {
            SkDebugf("finish callback all 3\n");
            fGpu->deleteFence(finishCallback.fFence);
        }
        SkDebugf("finish callback all 4\n");
        fCallbacks.pop_front();
        SkDebugf("finish callback all 5\n");
        finishCallback.fCallback(finishCallback.fContext);
        SkDebugf("finish callback all 6\n");
    }
    SkDebugf("finish callback all end\n");
}
