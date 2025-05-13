/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_vk_precompile_PrecompileShader_DEFINED
#define skgpu_graphite_vk_precompile_PrecompileShader_DEFINED

namespace PrecompileShaders {

/**
    In the main Skia API YCbCr Images are usually created by wrapping a BackendTexture.
    Such backend textures are, in turn, created by a backend-specific entry point like:
       BackendTextures::MakeVulkan(SkISize, const VulkanTextureInfo&, ...) or
       BackendTextures::MakeDawn(SkISize, const DawnTextureInfo&, ...)
    where the *TextureInfo contains additional YCbCr information. This API cuts right to
    providing the backend-specific YCbCr information (i.e., VulkanYcbcrConversionInfo
    or wgpu::YCbCrVkDescriptor).

    @return A precompile shader for a specific type of YCbCr image
*/
SK_API sk_sp<PrecompileShader> VulkanYCbCrImage(skgpu::VulkanYcbcrConversionInfo& YCbCrInfo,
                                                ImageShaderFlags = ImageShaderFlags::kAll,
                                                SkSpan<const SkColorInfo> = {},
                                                SkSpan<const SkTileMode> = { kAllTileModes });

} // namespace PrecompileShaders

#endif // skgpu_graphite_vk_precompile_PrecompileShader_DEFINED
