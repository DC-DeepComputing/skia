/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/cpu/Recorder.h"

#include "include/core/SkTypes.h"
#include "src/capture/SkCaptureManager.h"

#include <memory>

namespace skcpu {

Recorder::Recorder() = default;

Recorder::Recorder(sk_sp<SkCaptureManager> captureManager)
        : fCaptureManager(std::move(captureManager)) {}

Recorder::~Recorder() = default;

Recorder* Recorder::TODO() {
    static Recorder* gRecorder = std::make_unique<Recorder>().release();
    return gRecorder;
}

SkCanvas* Recorder::makeCaptureCanvas(SkCanvas* canvas) {
    if (fCaptureManager) {
        return fCaptureManager->makeCaptureCanvas(canvas);
    }
    return nullptr;
}

void Recorder::createCaptureBreakpoint(SkSurface* surface) {
    if (!fCaptureManager) {
        return;
    }
    if (auto picture = fCaptureManager->snapPicture(surface)) {
        skia_private::TArray<sk_sp<SkPicture>> captured;
        captured.push_back(std::move(picture));
        fCaptureManager->onInsertRecording(captured);
    }
}

void Recorder::deregisterCaptureCanvas(SkCanvas* canvas) {
    if (fCaptureManager) {
        fCaptureManager->deregisterCaptureCanvas(canvas);
    }
}

}  // namespace skcpu
