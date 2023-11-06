/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static initializers to work

#include "include/core/SkTypes.h"

#if defined(SK_GANESH) && defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/SkImageAndroid.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrAHardwareBufferImageGenerator.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include "tools/gpu/vk/AHardwareBufferTestUtils.h"

#include <android/hardware_buffer.h>
#include <cinttypes>

// There is no clean mapping from GrSurfaceOrigin to skgpu::Origin, so for now, just use a
// switch statement so we can utilize AHwBuffer utilities shared b/w ganesh and graphite.
namespace { // anonymous namespace

skgpu::Origin gr_surface_origin_to_skgpu_origin(GrSurfaceOrigin surfaceOrigin) {
    switch (surfaceOrigin) {
        case GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin: {
            return skgpu::Origin::kTopLeft;
        }
        case GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin: {
            return skgpu::Origin::kBottomLeft;
        }
        default: { SkUNREACHABLE; }
    }
}

} // anonymous namespace

static void basic_draw_test_helper(skiatest::Reporter* reporter,
                                   const sk_gpu_test::ContextInfo& info,
                                   GrSurfaceOrigin surfaceOrigin) {
    auto context = info.directContext();
    if (!context->priv().caps()->supportsAHardwareBufferImages()) {
        return;
    }

    // todo consolidate
    // There is no clean mapping from GrSurfaceOrigin to skgpu::Origin, so for now, just use a
    // switch statement so we can utilize AHwBuffer utilities shared b/w ganesh and graphite.


    ///////////////////////////////////////////////////////////////////////////
    // Setup SkBitmaps
    ///////////////////////////////////////////////////////////////////////////
    const SkBitmap srcBitmap = MakeSrcBitmap();
    ///////////////////////////////////////////////////////////////////////////
    // Setup AHardwareBuffer
    ///////////////////////////////////////////////////////////////////////////
    AHardwareBuffer* buffer = nullptr;
    AHardwareBuffer_Desc hwbDesc;
    SetupAHardwareBuffer(buffer, &hwbDesc, reporter);
    ///////////////////////////////////////////////////////////////////////////
    // Wrap AHardwareBuffer in SkImage
    ///////////////////////////////////////////////////////////////////////////
    sk_sp<SkImage> image = SkImages::DeferredFromAHardwareBuffer(
            buffer, kPremul_SkAlphaType, nullptr, surfaceOrigin);
    REPORTER_ASSERT(reporter, image);
    ///////////////////////////////////////////////////////////////////////////
    // Make a surface to draw into
    ///////////////////////////////////////////////////////////////////////////
    SkImageInfo imageInfo = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, imageInfo);
    REPORTER_ASSERT(reporter, surface);
    ///////////////////////////////////////////////////////////////////////////
    // Draw the AHardwareBuffer SkImage into surface
    ///////////////////////////////////////////////////////////////////////////
    surface->getCanvas()->drawImage(image, 0, 0);
    SkBitmap readbackBitmap;
    readbackBitmap.allocN32Pixels(DEV_W, DEV_H);
    REPORTER_ASSERT(reporter, surface->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, CheckRead(srcBitmap, readbackBitmap, reporter));
    // Draw the image a second time to make sure we get the correct origin when we get the cached
    // proxy from the generator.
    surface->getCanvas()->drawImage(image, 0, 0);
    REPORTER_ASSERT(reporter, surface->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, CheckRead(srcBitmap, readbackBitmap, reporter));
    image.reset();
    CleanupResources(buffer);
}


// Basic test to make sure we can import an AHardwareBuffer into an SkImage and draw it into a
// surface.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrAHardwareBuffer_BasicDrawTest,
                                       reporter,
                                       context_info,
                                       CtsEnforcement::kApiLevel_T) {
    basic_draw_test_helper(reporter, context_info, kTopLeft_GrSurfaceOrigin);
    basic_draw_test_helper(reporter, context_info, kBottomLeft_GrSurfaceOrigin);
}

static void surface_draw_test_helper(skiatest::Reporter* reporter,
                                     const sk_gpu_test::ContextInfo& info,
                                     GrSurfaceOrigin surfaceOrigin) {

    auto context = info.directContext();
    if (!context->priv().caps()->supportsAHardwareBufferImages()) {
        return;
    }

    const SkBitmap srcBitmap = MakeSrcBitmap();

    AHardwareBuffer* buffer = nullptr;
    AHardwareBuffer_Desc hwbDesc;
    SetupAHardwareBuffer(buffer, &hwbDesc, reporter);
//gr_surface_origin_to_skgpu_origin(surfaceOrigin),
    sk_sp<SkSurface> surface =
            SkSurfaces::WrapAndroidHardwareBuffer(context, buffer, surfaceOrigin, nullptr, nullptr);
    if (!surface) {
        ERRORF(reporter, "Failed to make SkSurface.");
        CleanupResources(buffer);
        return;
    }

    surface->getCanvas()->drawImage(srcBitmap.asImage(), 0, 0);

    SkBitmap readbackBitmap;
    readbackBitmap.allocN32Pixels(DEV_W, DEV_H);

    REPORTER_ASSERT(reporter, surface->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, CheckRead(srcBitmap, readbackBitmap, reporter));

    CleanupResources(buffer);
}

// Test to make sure we can import an AHardwareBuffer into an SkSurface and draw into it.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrAHardwareBuffer_ImportAsSurface,
                                       reporter,
                                       context_info,
                                       CtsEnforcement::kApiLevel_T) {
    surface_draw_test_helper(reporter, context_info, kTopLeft_GrSurfaceOrigin);
    surface_draw_test_helper(reporter, context_info, kBottomLeft_GrSurfaceOrigin);
}

#endif
