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

    enum class Type {
        kCPU,
        kGanesh,
        kGraphite,
    };

    virtual Type type() const = 0;

    virtual skcpu::Recorder* cpuRecorder() = 0;

    /** Allocates a bitmap-backed SkSurface. SkCanvas returned by SkSurface draws directly into
     *  those allocated pixels, which are zeroed before use. Pixel memory size is imageInfo.height()
     *  times imageInfo.minRowBytes() or rowBytes, if provided and non-zero.
     *
     *  Pixel memory is deleted when SkSurface is deleted.
     *
     *  Validity constraints include:
     *    - info dimensions are greater than zero;
     *    - info contains SkColorType and SkAlphaType supported by raster surface.
     *
     *  @param imageInfo  width, height, SkColorType, SkAlphaType, SkColorSpace,
     *                    of raster surface; width and height must be greater than zero
     *  @param rowBytes   interval from one SkSurface row to the next.
     *  @param props      LCD striping orientation and setting for device independent fonts;
     *                    may be nullptr
     *  @return           SkSurface if parameters are valid and memory was allocated, else nullptr.
     */
    sk_sp<SkSurface> makeBitmapSurface(const SkImageInfo& imageInfo,
                                       size_t rowBytes,
                                       const SkSurfaceProps* surfaceProps);
    sk_sp<SkSurface> makeBitmapSurface(const SkImageInfo& imageInfo,
                                       const SkSurfaceProps* surfaceProps = nullptr);

protected:
    skia_private::TArray<sk_sp<SkPicture>> fCapturedPictures;

private:

    /**
    * Attempts to create and return an SkCaptureCanvas that wraps the provided base canvas.
    * Returns nullptr if capture is not enabled.
    *
    * TODO: When implementing in all Recorder subclasses, see if the implementations can be shared
    * and moved here.
    */
    virtual SkCanvas* makeCaptureCanvas(SkCanvas*) = 0;
    virtual void createCaptureBreakpoint(SkSurface*) = 0;

    friend class SkSurface_Base; // for capture functionality
};

#endif
