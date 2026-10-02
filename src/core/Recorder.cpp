/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/cpu/Recorder.h"

#include "include/core/SkRecorder.h"
#include "include/core/SkTypes.h"
#include "src/capture/SkCaptureManager.h"

#include <memory>
#include <utility>

SkCanvas* SkRecorder::makeCaptureCanvas(SkCanvas* canvas) {
    if (SkCaptureManager* cm = this->captureManager()) {
        return cm->makeCaptureCanvas(canvas);
    }
    return nullptr;
}

void SkRecorder::createCaptureBreakpoint(SkSurface* surface) {
    if (SkCaptureManager* cm = this->captureManager()) {
        if (auto picture = cm->snapPicture(surface)) {
            fCapturedPictures.push_back(std::move(picture));
        }
    }
}

void SkRecorder::deregisterCaptureCanvas(SkCanvas* canvas) {
    if (SkCaptureManager* cm = this->captureManager()) {
        cm->deregisterCaptureCanvas(canvas);
    }
}

namespace skcpu {

Recorder::Recorder() = default;

Recorder::Recorder(sk_sp<SkCaptureManager> captureManager)
        : fCaptureManager(std::move(captureManager)) {}

Recorder::~Recorder() = default;

Recorder* Recorder::TODO() {
    static Recorder* gRecorder = std::make_unique<Recorder>().release();
    return gRecorder;
}

SkCaptureManager* Recorder::captureManager() { return fCaptureManager.get(); }

void Recorder::createCaptureBreakpoint(SkSurface* surface) {
    if (SkCaptureManager* cm = this->captureManager()) {
        if (auto picture = cm->snapPicture(surface)) {
            skia_private::TArray<sk_sp<SkPicture>> captured;
            captured.push_back(std::move(picture));
            cm->onInsertRecording(captured);
        }
    }
}

}  // namespace skcpu
