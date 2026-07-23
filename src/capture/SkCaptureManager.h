/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureManager_DEFINED
#define SkCaptureManager_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

#include <atomic>
#include <cstdint>
#include <map>

class SkCanvas;
class SkCapture;
class SkCaptureCanvas;
class SkSurface;

/**
 * SkCaptureManager is in charge of knowing the current state of capture, handling the creation of
 * capture canvases, and tracking and recording metadata to the final SkCapture.
 */
class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    SkCanvas* makeCaptureCanvas(SkCanvas* canvas);

    // TODO: Take in a SkPixelStorage ID instead
    sk_sp<SkPicture> snapPicture(SkSurface*);

    void toggleCapture(bool capturing);

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }

    skia_private::TArray<sk_sp<SkPicture>> captureDrawTasksForRecording();
    void onInsertRecording(const skia_private::TArray<sk_sp<SkPicture>>& capturedPictures);

    sk_sp<SkCapture> getLastCapture() const;

private:
    void captureUninsertedDrawTasks();
    sk_sp<SkPicture> snapAndIncrement(SkCaptureCanvas*);

    std::atomic<bool> fIsCurrentlyCapturing = false;
    skia_private::TArray<std::unique_ptr<SkCaptureCanvas>> fTrackedCanvases;
    sk_sp<SkCapture> fActiveCapture;

    sk_sp<SkCapture> fLastCapture;
};

#endif  // SkCaptureManager_DEFINED
