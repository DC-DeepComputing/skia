/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureManager_DEFINED
#define SkCaptureManager_DEFINED

#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"

#include <atomic>
#include <cstdint>
#include <memory>

class SkCanvas;
class SkCapture;
class SkSurface;

/**
 * SkCaptureManager is in charge of knowing the current state of capture, handling the creation of
 * capture canvases, and tracking and recording metadata to the final SkCapture.
 */
class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    void registerCanvas(SkCanvas* canvas);
    void unregisterCanvas(SkCanvas* canvas);

    void snapPictures();
    void snapPicture(SkSurface*);

    void toggleCapture(bool capturing);

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }

    sk_sp<SkCapture> getLastCapture() const;

private:
    struct TrackedCanvas {
        SkCanvas* fCanvas = nullptr;
        std::unique_ptr<SkPictureRecorder> fRecorder;
    };

    void startRecording(TrackedCanvas* tc);
    void stopRecording(TrackedCanvas* tc);

    std::atomic<bool> fIsCurrentlyCapturing = false;
    skia_private::TArray<TrackedCanvas> fTrackedCanvases;
    skia_private::TArray<sk_sp<SkPicture>> fPictures;

    sk_sp<SkCapture> fLastCapture;
};

#endif  // SkCaptureManager_DEFINED
