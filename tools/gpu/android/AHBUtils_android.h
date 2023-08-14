/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef AHBUtils_DEFINED
#define AHBUtils_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

struct AHardwareBuffer;

class GrDirectContext;
class SkBitmap;
class SkImage;
class SkSurface;

namespace AHBUtils {

AHardwareBuffer* CreateAHB(const SkBitmap& srcBitmap, GrSurfaceOrigin surfaceOrigin);

sk_sp<SkSurface> CreateAHBBackedSurface(GrDirectContext* dContext,
                                        int width, int height,
                                        GrSurfaceOrigin surfaceOrigin,
                                        bool isProtected,
                                        AHardwareBuffer** ahb);

sk_sp<SkImage> CreateAHBBackedImage(GrDirectContext* dContext,
                                    const SkBitmap& srcBitmap,
                                    GrSurfaceOrigin surfaceOrigin,
                                    bool isProtected,
                                    AHardwareBuffer** ahb);

} // namespace AHBUtils

#endif

#endif  // AHBUtils_DEFINED
