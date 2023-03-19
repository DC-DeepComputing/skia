/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "src/core/SkBitmapDevice.h"
#include "src/text/GlyphRun.h"

SkCanvas::SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)), fProps(props) {
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap,
                   std::unique_ptr<SkRasterHandleAllocator> alloc,
                   SkRasterHandleAllocator::Handle hndl,
                   const SkSurfaceProps* props)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage))
        , fProps(SkSurfacePropsCopyOrDefault(props))
        , fAllocator(std::move(alloc)) {
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, fProps, hndl));
    this->init(device);
}

SkCanvas::SkCanvas(const SkBitmap& bitmap) : SkCanvas(bitmap, nullptr, nullptr, nullptr) {}

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
SkCanvas::SkCanvas(const SkBitmap& bitmap, ColorBehavior)
        : fMCStack(sizeof(MCRec), fMCRecStorage, sizeof(fMCRecStorage)) {
    SkBitmap tmp(bitmap);
    *const_cast<SkImageInfo*>(&tmp.info()) = tmp.info().makeColorSpace(nullptr);
    sk_sp<SkBaseDevice> device(new SkBitmapDevice(tmp, fProps));
    this->init(device);
}
#endif

