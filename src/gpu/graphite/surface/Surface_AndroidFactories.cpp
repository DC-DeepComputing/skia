/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if __ANDROID_API__ >= 26

#include "include/android/AHardwareBufferUtils.h"
#include "include/android/graphite/SurfaceAndroid.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

#include <android/hardware_buffer.h>

using namespace skgpu::graphite;

namespace SkSurfaces {

sk_sp<SkSurface> WrapAndroidHardwareBuffer(Recorder* recorder,
                                           AHardwareBuffer* hardwareBuffer,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* surfaceProps,
                                           BufferReleaseProc releaseP,
                                           ReleaseContext releaseC,
                                           bool fromWindow) {
    if (!recorder || !hardwareBuffer) {
        return nullptr;
    }

    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT) ||
        !SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        if (releaseP) {
            releaseP(releaseC);
        }
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
        if (releaseP) {
            releaseP(releaseC);
        }
        return nullptr;
    }

    // BackendTextures based on AHardwareBuffers in Graphite should use
    // VulkanBackendTextureData::getSkColorType rather than simply getting the SkColorType
    // from `AHardwareBufferUtils::GetSkColorTypeFromBufferFormat` (which is shared with Ganesh)
    // because Graphite has additional checking and format support.
    const bool textureUsesExternalFormat =
            TextureInfoPriv::ViewFormat(backendTexture.info()) == TextureFormat::kExternal;
    const SkColorType ct =
            BackendTexturePriv::GetData(backendTexture)->getSkColorType(textureUsesExternalFormat);

    // Ensure that we have not somehow ended up in a situation where the determined color type is
    // incompatible with the texture's format.
    SkASSERT(textureUsesExternalFormat ||
             recorder->priv().caps()->areColorTypeAndTextureInfoCompatible(ct,
                                                                           backendTexture.info()));

    // Will call 'releaseP' if SkSurface creation fails.
    return SkSurfaces::WrapBackendTexture(recorder,
                                          backendTexture,
                                          ct,
                                          std::move(colorSpace),
                                          surfaceProps,
                                          releaseP,
                                          releaseC);
}

}  // namespace SkSurfaces

#endif // __ANDROID_API__ >= 26
