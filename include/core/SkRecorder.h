/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRecorder_DEFINED
#define SkRecorder_DEFINED

#include "include/core/SkPicture.h"
#include "include/private/SkAPI.h"
#include "include/private/SkTArray.h"

class SkCanvas;
class SkCaptureManager;
class SkSurface;

namespace skcpu {
class Recorder;
}

class SK_API SkRecorder {
public:
    SkRecorder() = default;
    virtual ~SkRecorder() = default;
    SkRecorder(const SkRecorder&) = delete;
    SkRecorder(SkRecorder&&) = delete;
    SkRecorder& operator=(const SkRecorder&) = delete;
    SkRecorder& operator=(SkRecorder&&) = delete;

    enum class Type {
        kCPU,
        kGanesh,
        kGraphite,
    };

    virtual Type type() const = 0;

    virtual skcpu::Recorder* cpuRecorder() = 0;

protected:
    virtual SkCaptureManager* captureManager() { return nullptr; }
    virtual SkCanvas* makeCaptureCanvas(SkCanvas*);
    virtual void createCaptureBreakpoint(SkSurface*);
    virtual void deregisterCaptureCanvas(SkCanvas*);

    skia_private::TArray<sk_sp<SkPicture>> fCapturedPictures;

private:
    friend class SkSurface_Base; // for capture functionality
};

#endif
