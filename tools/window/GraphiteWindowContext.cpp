/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/window/GraphiteWindowContext.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"

namespace skwindow::internal {

GraphiteWindowContext::GraphiteWindowContext(sk_sp<DisplayParams> dp) : WindowContext(dp) {}

void GraphiteWindowContext::snapRecordingAndSubmit() {
    if (fGraphiteContext) {
        SkASSERT(fGraphiteRecorder);
        std::unique_ptr<skgpu::graphite::Recording> recording = fGraphiteRecorder->snap();
        if (recording) {
            skgpu::graphite::InsertRecordingInfo info;
            info.fRecording = recording.get();
            fGraphiteContext->insertRecording(info);
            fGraphiteContext->submit(skgpu::graphite::SyncToCpu::kNo);
        }
    }
}

}  // namespace skwindow::internal
