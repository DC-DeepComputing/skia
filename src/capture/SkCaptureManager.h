/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureManager_DEFINED
#define SkCaptureManager_DEFINED

#include <memory>
#include <vector>
#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "src/capture/SkCaptureCanvas.h"


class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    SkCaptureCanvas* makeCaptureCanvas(SkCanvas* canvas);

    void toggleCapture(bool capturing) {
        fIsCurrentlyCapturing = capturing;
    }

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }


private:
    bool fIsCurrentlyCapturing = false;
    std::vector<std::unique_ptr<SkCaptureCanvas>> fTrackedCanvases;
};

#endif  // SkCaptureManager_DEFINED
