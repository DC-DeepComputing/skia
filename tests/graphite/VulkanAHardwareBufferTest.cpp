/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/vk/AHardwareBufferVulkan.h"

#include <android/hardware_buffer.h>
#include <cinttypes>

using namespace skgpu::graphite;

namespace {
const SkISize kSize = {16, 16};

void cleanup_resources(AHardwareBuffer* buffer) {
    if (buffer) {
        AHardwareBuffer_release(buffer);
    }
}

SkPMColor get_src_color(int x, int y) {
    SkASSERT(x >= 0 && x < kSize.fWidth);
    SkASSERT(y >= 0 && y < kSize.fHeight);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0xff;
    switch ((x+y) % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 4:
            a = 0x01;
            break;
        case 3:
            a = 0x00;
            break;
    }
    a = 0xff;
    return SkPremultiplyARGBInline(a, r, g, b);
}

SkBitmap make_src_bitmap() {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.allocN32Pixels(kSize.fWidth, kSize.fHeight);
        intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
        for (int y = 0; y < kSize.fHeight; ++y) {
            for (int x = 0; x < kSize.fWidth; ++x) {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(
                        pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
                *pixel = get_src_color(x, y);
            }
        }
    }
    return bmp;
}

static bool check_read(skiatest::Reporter* reporter, const SkBitmap& expectedBitmap,
                       const SkBitmap& actualBitmap) {
    bool result = true;
    for (int y = 0; y < kSize.fHeight && result; ++y) {
        for (int x = 0; x < kSize.fWidth && result; ++x) {
            const uint32_t srcPixel = *expectedBitmap.getAddr32(x, y);
            const uint32_t dstPixel = *actualBitmap.getAddr32(x, y);
            if (srcPixel != dstPixel) {
                ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x.",
                       x, y,  srcPixel, dstPixel);
                result = false;
            }/* else {
                SkDebugf("Got good pixel (%d, %d) value 0x%08x, got 0x%08x.\n",
                       x, y,  srcPixel, dstPixel);
            }*/
        }
    }
    return result;
}
}

// Basic test to make sure we can import an AHardwareBuffer into an SkImage and draw it into a
// surface.
// DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS()


// Test to make sure we can import an AHardwareBuffer into an SkSurface and draw into it.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(VulkanImportAHardwareBufferTest, reporter, context) {
    // Setup source SkBitmap, AHardwareBuffer
    const SkBitmap srcBitmap = make_src_bitmap();
    AHardwareBuffer* buffer = nullptr;

    AHardwareBuffer_Desc hwbDesc;
    hwbDesc.width = kSize.fWidth;
    hwbDesc.height = kSize.fHeight;
    hwbDesc.layers = 1;
    hwbDesc.usage = AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
                    AHARDWAREBUFFER_USAGE_CPU_WRITE_NEVER |
                    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                    AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

    hwbDesc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    // The following three are not used in the allocate
    hwbDesc.stride = 0;
    hwbDesc.rfu0= 0;
    hwbDesc.rfu1= 0;

    if (int error = AHardwareBuffer_allocate(&hwbDesc, &buffer)) {
        ERRORF(reporter, "Failed to allocated hardware buffer, error: %d", error);
        cleanup_resources(buffer);
        return;
    }

    // Can we simply do: GetVulkanTextureInfo (if needed), MakeVulkanBackendTexture, then
    // Surface::WrapBackendTexture instead of having a whole SurfaceAndroid / factory to do so?
    VulkanTextureInfo vulkanTextureInfo =
            GetVulkanTextureInfo(context, buffer, hwbDesc.format, false);

    DeleteImageProc deleteImageProc = nullptr;
    UpdateImageProc updateImageProc = nullptr;
    TexImageCtx deleteImageCtx = nullptr;
    BackendTexture backendTexture = MakeVulkanBackendTexture(context,
                                                             buffer,
                                                             hwbDesc.width, hwbDesc.height,
                                                             deleteImageProc,
                                                             updateImageProc,
                                                             deleteImageCtx,
                                                             /*isProtectedContent=*/false,
                                                             vulkanTextureInfo,
                                                             /*isRenderable=*/false);

    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                              backendTexture,
                                                              kAlpha_8_SkColorType,
                                                              /*colorSpace=*/nullptr,
                                                              /*props=*/nullptr);
    if (!surface) {
        ERRORF(reporter, "Failed to make SkSurface.");
        cleanup_resources(buffer);
        return;
    }

    // canvas, surface?
    // Ganesh:
    //     sk_sp<SkSurface> surface =
    //         SkSurfaces::WrapAndroidHardwareBuffer(context, buffer, surfaceOrigin, nullptr, nullptr);
    // if (!surface) {
    //     ERRORF(reporter, "Failed to make SkSurface.");
    //     cleanup_resources(buffer);
    //     return;
    // }

    surface->getCanvas()->drawImage(srcBitmap.asImage(), 0, 0);

    SkBitmap readbackBitmap;
    readbackBitmap.allocN32Pixels(kSize.fWidth, kSize.fHeight);

    //asyncReadPixels?
    //REPORTER_ASSERT(reporter, surface->readPixels(readbackBitmap, 0, 0));
    REPORTER_ASSERT(reporter, check_read(reporter, srcBitmap, readbackBitmap));

    cleanup_resources(buffer);
}

#endif
