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

struct AHardwareBuffer_Desc;

// AHardwareBuffer_acquire
// AHardwareBuffer_allocate
// AHardwareBuffer_release
class AHardwareBuffer2 {
public:
    int refCnt() const { return fRefCount; }
    AHardwareBuffer* buffer1() { return fBuffer; }

    bool allocate(const AHardwareBuffer_Desc& hwbDesc);
    void acquire();
    void release();

private:
    AHardwareBuffer* fBuffer = nullptr;
    int fRefCount = 0;
};

namespace AHBUtils {

AHardwareBuffer2* CreateAHB(const SkBitmap& srcBitmap, GrSurfaceOrigin surfaceOrigin);

sk_sp<SkSurface> CreateAHBBackedSurface(GrDirectContext* dContext,
                                        int width, int height,
                                        GrSurfaceOrigin surfaceOrigin,
                                        bool isProtected);

sk_sp<SkImage> CreateAHBBackedImage(GrDirectContext* dContext,
                                    const SkBitmap& srcBitmap,
                                    GrSurfaceOrigin surfaceOrigin,
                                    bool isProtected);

} // namespace AHBUtils

#endif

#endif  // AHBUtils_DEFINED
