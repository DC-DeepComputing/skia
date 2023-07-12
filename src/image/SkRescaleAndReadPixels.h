/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkImage.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"

class SkBitmap;
struct SkIRect;
struct SkImageInfo;

namespace SkRescaleAndReadPixelsUtils {

/** Generic/synchronous implementation for SkImage:: and SkSurface::asyncRescaleAndReadPixels. */
void SynchronousRescaleAndReadPixels(SkBitmap src,
                                     const SkImageInfo& resultInfo,
                                     const SkIRect& srcRect,
                                     SkImage::RescaleGamma,
                                     SkImage::RescaleMode,
                                     SkImage::RGBAReadPixelsCallback,
                                     SkImage::ReadPixelsContext);

bool IsYUVAInfoValid(const SkYUVAInfo&, const SkYUVAPixmapInfo::DataType);

struct LegacyWrapperContext {
    SkImage::ReadPixelsCallback* fClientCallback;
    SkImage::ReadPixelsContext   fClientContext;
};

void LegacyRGBACallbackWrapper(SkImage::ReadPixelsContext, std::unique_ptr<const SkImage::RGBAAsyncReadPixelsResult>);
void LegacyYUVACallbackWrapper(SkImage::ReadPixelsContext, std::unique_ptr<const SkImage::YUVAAsyncReadPixelsResult>);

}
