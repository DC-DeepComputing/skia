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

Recorder* Recorder::TODO() {
    static Recorder* gRecorder = std::make_unique<Recorder>().release();
    return gRecorder;
}

void Recorder::createCaptureBreakpoint(SkSurface*) {
}

}  // namespace skcpu
