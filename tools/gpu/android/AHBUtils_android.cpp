/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/android/AHBUtils_android.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/SkSurfaceAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"

#include <android/hardware_buffer.h>

namespace {

static void cleanup_resources(AHardwareBuffer* buffer) {
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

} // anonymous namespace

namespace AHBUtils {

sk_sp<SkSurface> CreateAHBBackedSurface(GrDirectContext* dContext,
                                        int width, int height,
                                        GrSurfaceOrigin surfaceOrigin,
                                        bool isProtected,
                                        AHardwareBuffer** backingAHB) {
    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = width;
    hwbDesc.height = height;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                    AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

    if (isProtected) {
        SkASSERT(dContext->priv().caps()->supportsAHardwareBufferImages());
        // TODO: add an assert that the direct context supports protected content
        hwbDesc.usage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
    }

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, backingAHB)) {
        SkDebugf("Failed to allocated hardware buffer, error: %d\n", error);
        cleanup_resources(*backingAHB);
        *backingAHB = nullptr;
        return nullptr;
    }

    sk_sp<SkSurface> surface = SkSurfaces::WrapAndroidHardwareBuffer(dContext, *backingAHB,
                                                                     surfaceOrigin,
                                                                     /* colorSpace= */ nullptr,
                                                                     /* surfaceProps= */ nullptr);
    if (!surface) {
        SkDebugf("Failed to make SkSurface.\n");
        cleanup_resources(*backingAHB);
        *backingAHB = nullptr;
        return nullptr;
    }

    return surface;
}

sk_sp<SkImage> CreateAHBBackedImage(GrDirectContext* dContext,
                                    const SkBitmap& srcBitmap,
                                    GrSurfaceOrigin surfaceOrigin,
                                    bool isProtected,
                                    AHardwareBuffer** backingAHB) {
    sk_sp<SkSurface> surf = CreateAHBBackedSurface(dContext, srcBitmap.width(), srcBitmap.height(),
                                                   surfaceOrigin, isProtected, backingAHB);
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();

    SkPaint paint;
    paint.setShader(srcBitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                         SkSamplingOptions()));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);

    return surf->makeImageSnapshot();
}

} // namespace AHBUtils

#endif
