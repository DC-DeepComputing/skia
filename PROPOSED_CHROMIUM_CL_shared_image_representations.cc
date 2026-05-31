/*
 * Copyright 2026 The Chromium Authors
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Proposed change to chromium\src\gpu\command_buffer\service\shared_image\shared_image_representation.cc

//we will need to edit both the code sections for skia GRAPHITE and SKIA GANESH

//we will need to add new matrices in SkYUVMath.cpp, e.g. kBT2020_10bitSampledAs16_Limited_SkYUVColorSpace
//do so for 10 bit + 12 bit + limited + full


sk_sp<SkImage>
SkiaGraphiteImageRepresentation::ScopedReadAccess::CreateSkImage(
    skgpu::graphite::Recorder* recorder,
    SkImages::TextureReleaseProc wrapped_release_proc,
    SkImages::ReleaseContext wrapped_release_context) {
  auto format = representation()->format();

  if (format.PrefersExternalSampler()) {
    // -------------------------------------------------------------------------
    // BRANCH A: External Sampler (Used by Android and native Vulkan)
    // -------------------------------------------------------------------------
    // If external sampling is preferred, the GPU handles YUV-to-RGB internally.
    // Skia does not construct a SkYUVAInfo or use Skia's YUV-to-RGB matrices here.
    // Android always enters this branch.
    ...
    return SkImages::WrapTexture(recorder, graphite_texture(), ...);

  } else {
    // -------------------------------------------------------------------------
    // BRANCH B: Plane-Splitting (Used by Windows and macOS Graphite fallback)
    // -------------------------------------------------------------------------
    // Here, Chromium splits the YUV frame into separate plane textures,
    // and Skia performs the color space conversion inside the shader.

    SkISize sk_size = gfx::SizeToSkISize(representation()->size());

    SkYUVColorSpace yuv_color_space = kRec601_SkYUVColorSpace;
    representation()->color_space().ToSkYUVColorSpace(
        format.MultiplanarBitDepth(), &yuv_color_space);

#if !BUILDFLAG(IS_APPLE)
    // -------------------------------------------------------------------------
    // 16-BIT UNORM SAMPLING CORRECTION for 10/12 bit video (Applies to Windows Direct3D 11/12, Vulkan, and OpenGL):
    // -------------------------------------------------------------------------
    //
    // Direct3D, Vulkan, and OpenGL do not allow binding a planar 10-bit YUV (P010) texture
    // directly to a standard 2D shader sampler. The APIs mandate creating separate views for each plane:
    //   - Y plane bound as DXGI_FORMAT_R16_UNORM / VK_FORMAT_R16_UNORM / GL_R16_EXT
    //   - UV plane bound as DXGI_FORMAT_R16G16_UNORM / VK_FORMAT_R16G16_UNORM / GL_RG16_EXT
    //
    // Direct3D (11/12), Vulkan, and OpenGL specifications mandate that standard
    // 16-bit UNORM texture views (like R16_UNORM and RG16_UNORM used for plane-splitting)
    // must be normalized by dividing the integer value by exactly 65535.
    //
    // Because these are standard 16-bit UNORM texture views, there is NO metadata
    // or API-level flags to tell the GPU sampler that the underlying content is e.g.
    // 10-bit. The GPU hardware sampler has no choice but to treat the memory as
    // standard 16-bit and divide by 65535, causing a normalization mismatch
    // (max value is 65472 instead of 65535).
    //
    // Unlike Apple's Metal + IOSurface pipeline, these APIs/drivers do not automatically
    // associate the plane views with the parent video buffer metadata to scale the
    // normalization denominator to the 10-bit range (dividing by 65472 instead).
    // This results in a normalization mismatch (max value is 65472/65535 ≈ 0.999
    // instead of 1.0), which causes a visible color shift (green tint).
    //
    // To correct this API/hardware behavior on non-Apple platforms, we map the
    // color space to a "SampledAs16" variant with scaled matrix coefficients.
    //
    // NOTE: If a future API or extension on Windows/Linux (e.g., a native multi-planar
    // sampling extension or metadata-aware views) enables the driver to normalize
    // these views correctly, we should bypass this correction for those paths.
    // -------------------------------------------------------------------------
    if (format.MultiplanarBitDepth() == 10) {
        if (yuv_color_space == kBT2020_10bit_Limited_SkYUVColorSpace) {
            yuv_color_space = kBT2020_10bitSampledAs16_Limited_SkYUVColorSpace;
        } else if (yuv_color_space == kBT2020_10bit_Full_SkYUVColorSpace) {
            yuv_color_space = kBT2020_10bitSampledAs16_Full_SkYUVColorSpace;
        }
    } else if (format.MultiplanarBitDepth() == 12) {
        if (yuv_color_space == kBT2020_12bit_Limited_SkYUVColorSpace) {
            yuv_color_space = kBT2020_12bitSampledAs16_Limited_SkYUVColorSpace;
        } else if (yuv_color_space == kBT2020_12bit_Full_SkYUVColorSpace) {
            yuv_color_space = kBT2020_12bitSampledAs16_Full_SkYUVColorSpace;
        }
    }
#else
    // -------------------------------------------------------------------------
    // APPLE (Metal + IOSurface) BEHAVIOR:
    // -------------------------------------------------------------------------
    // Although Apple also wraps the split planes using MTLPixelFormatR16Unorm,
    // the Metal texture object remains linked to the original IOSurface object.
    // The IOSurface retains OS-level metadata indicating it contains a native
    // 10/12-bit video format (e.g. 'x420' for 10-bit).
    //
    // Apple's Metal driver reads this IOSurface metadata and automatically
    // configures the GPU sampler to divide by the correct native bit-depth
    // range (e.g., 65472 for 10-bit or 65520 for 12-bit). Thus, the shader receives
    // correctly normalized [0.0, 1.0] floats out-of-the-box, requiring NO correction.
    // -------------------------------------------------------------------------
#endif

    SkYUVAInfo yuva_info(sk_size, ToSkYUVAPlaneConfig(format),
                         ToSkYUVASubsampling(format), yuv_color_space);

    std::vector<skgpu::graphite::BackendTexture> backend_textures(
        graphite_texture_holders_.size());
    for (int i = 0; i < format.NumberOfPlanes(); ++i) {
      backend_textures[i] = graphite_texture_holders_[i]->texture();
    }
    skgpu::graphite::YUVABackendTextures yuva_backend_textures(
        yuva_info, backend_textures);
    return SkImages::TextureFromYUVATextures(
        recorder, yuva_backend_textures, ...);
  }
}
