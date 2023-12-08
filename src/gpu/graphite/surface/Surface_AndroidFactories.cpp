/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/graphite/SurfaceAndroid.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/android/AHardwareBufferUtils.h"
#include "include/gpu/graphite/Surface.h"

#include <android/hardware_buffer.h>

using namespace skgpu::graphite;

#if 0
namespace {

class Foo {
public:
    Foo() {

    }
private:

};

void delete_vk_image(void* context) {
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    delete cleanupHelper;
}

} // anonymous namespace
#endif

namespace SkSurfaces {

sk_sp<SkSurface> WrapAndroidHardwareBuffer(Recorder* recorder,
                                           AHardwareBuffer* hardwareBuffer,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* surfaceProps,
                                           bool fromWindow) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT) ||
        !SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        return nullptr;
    }

    bool isProtectedContent = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

    bool fromWindowLocal = false;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    fromWindowLocal = fromWindow;
#endif

    SkISize dims = SkISize::Make(bufferDesc.width, bufferDesc.height);

    BackendTexture backendTexture = recorder->createBackendTexture(hardwareBuffer,
                                                                   /* isRenderable= */ true,
                                                                   isProtectedContent,
                                                                   dims,
                                                                   fromWindowLocal);
    if (!backendTexture.isValid()) {
        return nullptr;
    }

//    Foo foo();

    SkColorType colorType =
            AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    // Will call delete_vk_backend_texture if SkSurface creation fails.
    return SkSurfaces::WrapBackendTexture(recorder,
                                          backendTexture,
                                          colorType,
                                          std::move(colorSpace),
                                          surfaceProps,
                                          /* textureReleaseProc= */ nullptr,
                                          /* textureReleaseContext= */ nullptr);
}

}  // namespace SkSurfaces

#endif
