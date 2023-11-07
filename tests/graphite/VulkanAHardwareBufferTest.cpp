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
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"

#include "tools/gpu/vk/AHardwareBufferTestUtils.h"

#include <android/hardware_buffer.h>
#include <cinttypes>

using namespace skgpu::graphite;

// Basic test to make sure we can import an AHardwareBuffer into an SkImage and draw it into a
// surface.
// DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS()

// TODO: Make this test only run when we are building for Android, Vulkan
// Test to make sure we can import an AHardwareBuffer into an SkSurface and draw into it.
DEF_GRAPHITE_TEST_FOR_VULKAN_CONTEXT(VulkanImportAHardwareBufferTest, reporter, context,
                                     CtsEnforcement::kNextRelease) {
    // Setup source SkBitmap, AHardwareBuffer
    // TODO test w/ both origin options
    AHardwareBuffer* buffer = nullptr;
    AHardwareBuffer_Desc hwbDesc;
    SetupAHardwareBuffer(buffer, &hwbDesc, reporter);

    const SkBitmap srcBitmap = MakeSrcBitmap();
    PopulateBitmap(srcBitmap, buffer, hwbDesc.stride, skgpu::Origin::kTopLeft, reporter);

    // Now create and use a recorder from the context to create a BackendTexture
    auto recorder = context->makeRecorder();

    // TODO: Test this capability with protected content once we support protected contexts within
    // graphite.
    TextureInfo texInfo =
            recorder->getTextureInfoFromAHardwareBuffer(buffer,
                                                        hwbDesc.format,
                                                        /*requireKnownFormat=*/true,
                                                        /*isRenderable=*/true,
                                                        /*isProtectedContent=*/false);
    VulkanTextureInfo vulkanTextureInfo;
    if (!texInfo.isValid() || !texInfo.getVulkanTextureInfo(&vulkanTextureInfo)) {
        ERRORF(reporter, "Failed to obtain Vulkan texture info");
        CleanupResources(buffer);
        return;
    }

    BackendTexture backendTexture = recorder->createBackendTexture(buffer,
                                                                   hwbDesc.width,
                                                                   hwbDesc.height,
                                                                   vulkanTextureInfo);
    REPORTER_ASSERT(reporter, backendTexture.isValid());
    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(recorder.get(),
                                                              backendTexture,
                                                              kRGBA_8888_SkColorType,
                                                              /*colorSpace=*/nullptr,
                                                              /*props=*/nullptr);
    if (!surface) {
        ERRORF(reporter, "Failed to make SkSurface.");
        CleanupResources(buffer);
        return;
    }

    DrawImageAndReadBack(srcBitmap, *surface.get(), reporter);

    CleanupResources(buffer);
    recorder->deleteBackendTexture(backendTexture);
}

#endif
