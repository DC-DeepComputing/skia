/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_AHardwareBufferVulkan_DEFINED
#define skgpu_graphite_AHardwareBufferVulkan_DEFINED

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
}

namespace skgpu::graphite {

class VulkanSharedContext;

TextureInfo GetVulkanTextureInfo(Context*, AHardwareBuffer*, uint32_t bufferFormat,
                                 bool requireKnownFormat);

typedef void* TexImageCtx;
typedef void (*DeleteImageProc)(TexImageCtx);
typedef void (*UpdateImageProc)(TexImageCtx, Context*);

/**
 * Create a BackendTexture from AHardwareBuffer
 *
 * @param   context            GPU context
 * @param   hardwareBuffer     AHB
 * @param   width              texture width
 * @param   height             texture height
 * @param   deleteProc         returns a function that deletes the texture and
 *                             other GPU resources. Must be invoked on the same
 *                             thread as MakeBackendTexture
 * @param   updateProc         returns a function, that needs to be invoked, when
 *                             AHB buffer content has changed. Must be invoked on
 *                             the same thread as MakeBackendTexture
 * @param   imageCtx           returns an opaque image context, that is passed as
 *                             first argument to deleteProc and updateProc
 * @param   isProtectedContent if true, GL backend uses EXT_protected_content. TODO: Still relevant?
 * @param   backendFormat      backend format, usually created with helper
 *                             function GetBackendTextureInfo
 * @param   isRenderable       true if BackendTexture can be used as a color
 *                             attachment
 * @return                     valid BackendTexture object on success
 */
BackendTexture MakeVulkanBackendTexture(Context*,
                                        AHardwareBuffer*,
                                        int width, int height,
                                        DeleteImageProc*,
                                        UpdateImageProc*,
                                        TexImageCtx* imageCtx,
                                        bool isProtectedContent,
                                        const TextureInfo& backendTextureInfo,
                                        bool isRenderable,
                                        bool fromAndroidWindow = false);

} // skgpu:: graphite
#endif
#endif
