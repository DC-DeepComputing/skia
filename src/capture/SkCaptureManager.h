/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCaptureManager_DEFINED
#define SkCaptureManager_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"

#include <atomic>

class SkCanvas;
class SkCaptureCanvas;
class SkSurface;

class SkCaptureManager : public SkRefCnt {
public:
    SkCaptureManager();

    SkCanvas* makeCaptureCanvas(SkCanvas* canvas);
    void snapPictures();
    void snapPicture(SkSurface*);
    //TODO: move serialize capture to SkCapture
    sk_sp<SkData> serializeCapture();
    // naive proc for serializing images
    // TODO: add more awareness of the image meta data to a SkCaptureContext object
    static sk_sp<SkData> serializeImageProc(SkImage* img, void* ctx);
    static sk_sp<SkImage> deserializeImageProc(sk_sp<SkData>, std::optional<SkAlphaType>, void* ctx);

    void toggleCapture(bool capturing) {
        if (capturing != fIsCurrentlyCapturing && !capturing) {
            this->snapPictures();
        }
        fIsCurrentlyCapturing = capturing;
    }

    bool isCurrentlyCapturing() const {
        return fIsCurrentlyCapturing;
    }

private:
    std::atomic<bool> fIsCurrentlyCapturing = false;
    skia_private::TArray<std::unique_ptr<SkCaptureCanvas>> fTrackedCanvases;
    skia_private::TArray<sk_sp<SkPicture>>  fPictures;
};

#endif  // SkCaptureManager_DEFINED
