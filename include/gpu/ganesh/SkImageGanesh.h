/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGanesh_DEFINED
#define SkImageGanesh_DEFINED

#include "include/core/SkImage.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"

class SK_API SkImageGanesh : public SkImage {
public:
    /** Defines a callback function, taking one parameter of type GrBackendTexture with
        no return value. Function is called when back-end texture is to be released.
    */
    using BackendTextureReleaseProc = std::function<void(GrBackendTexture)>;

    static bool MakeBackendTextureFromSkImage(GrDirectContext* context,
                                              sk_sp<SkImage> image,
                                              GrBackendTexture* backendTexture,
                                              BackendTextureReleaseProc* backendTextureReleaseProc);

    /** User function called when supplied texture may be deleted.
    */
    typedef void (*TextureReleaseProc)(ReleaseContext releaseContext);

    /** Creates SkImage from GPU texture associated with context. GPU texture must stay
        valid and unchanged until textureReleaseProc is called. textureReleaseProc is
        passed releaseContext when SkImage is deleted or no longer refers to texture.
        SkImage is returned if format of backendTexture is recognized and supported.
        Recognized formats vary by GPU back-end.
        @note When using a DDL recording context, textureReleaseProc will be called on the
        GPU thread after the DDL is played back on the direct context.
        @param context             GPU context
        @param backendTexture      texture residing on GPU
        @param colorSpace          This describes the color space of this image's contents, as
                                   seen after sampling. In general, if the format of the backend
                                   texture is SRGB, some linear colorSpace should be supplied
                                   (e.g., SkColorSpace::MakeSRGBLinear()). If the format of the
                                   backend texture is linear, then the colorSpace should include
                                   a description of the transfer function as
                                   well (e.g., SkColorSpace::MakeSRGB()).
        @param textureReleaseProc  function called when texture can be released
        @param releaseContext      state passed to textureReleaseProc
        @return                    created SkImage, or nullptr
    */
    static sk_sp<SkImageGanesh> MakeFromTexture(GrRecordingContext* context,
                                                const GrBackendTexture& backendTexture,
                                                GrSurfaceOrigin origin,
                                                SkColorType colorType,
                                                SkAlphaType alphaType,
                                                sk_sp<SkColorSpace> colorSpace,
                                                TextureReleaseProc textureReleaseProc = nullptr,
                                                ReleaseContext releaseContext = nullptr);

    using PromiseImageTextureContext = void*;
    using PromiseImageTextureFulfillProc =
            sk_sp<SkPromiseImageTexture> (*)(PromiseImageTextureContext);
    using PromiseImageTextureReleaseProc = void (*)(PromiseImageTextureContext);

    /** Create a new SkImage that is very similar to an SkImage created by MakeFromTexture. The
        difference is that the caller need not have created the texture nor populated it with the
        image pixel data. Moreover, the SkImage may be created on a thread as the creation of the
        image does not require access to the backend API or GrDirectContext. Instead of passing a
        GrBackendTexture the client supplies a description of the texture consisting of
        GrBackendFormat, width, height, and GrMipmapped state. The resulting SkImage can be drawn
        to a SkDeferredDisplayListRecorder or directly to a GPU-backed SkSurface.
        When the actual texture is required to perform a backend API draw, textureFulfillProc will
        be called to receive a GrBackendTexture. The properties of the GrBackendTexture must match
        those set during the SkImage creation, and it must refer to a valid existing texture in the
        backend API context/device, and be populated with the image pixel data. The texture cannot
        be deleted until textureReleaseProc is called.
        There is at most one call to each of textureFulfillProc and textureReleaseProc.
        textureReleaseProc is always called even if image creation fails or if the
        image is never fulfilled (e.g. it is never drawn or all draws are clipped out)
        @param gpuContextProxy     the thread-safe proxy of the gpu context. required.
        @param backendFormat       format of promised gpu texture
        @param dimensions          width & height of promised gpu texture
        @param mipmapped           mip mapped state of promised gpu texture
        @param origin              surface origin of promised gpu texture
        @param colorType           color type of promised gpu texture
        @param alphaType           alpha type of promised gpu texture
        @param colorSpace          range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be deleted
        @param textureContext      state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
    */
    static sk_sp<SkImageGanesh> MakePromiseTexture(
                                             sk_sp<GrContextThreadSafeProxy> gpuContextProxy,
                                             const GrBackendFormat& backendFormat,
                                             SkISize dimensions,
                                             GrMipmapped mipmapped,
                                             GrSurfaceOrigin origin,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             PromiseImageTextureFulfillProc textureFulfillProc,
                                             PromiseImageTextureReleaseProc textureReleaseProc,
                                             PromiseImageTextureContext textureContext);

    /** This entry point operates like 'MakePromiseTexture' but it is used to construct a SkImage
        from YUV[A] data. The source data may be planar (i.e. spread across multiple textures). In
        the extreme Y, U, V, and A are all in different planes and thus the image is specified by
        four textures. 'backendTextureInfo' describes the planar arrangement, texture formats,
        conversion to RGB, and origin of the textures. Separate 'textureFulfillProc' and
        'textureReleaseProc' calls are made for each texture. Each texture has its own
        PromiseImageTextureContext. If 'backendTextureInfo' is not valid then no release proc
        calls are made. Otherwise, the calls will be made even on failure. 'textureContexts' has one
        entry for each of the up to four textures, as indicated by 'backendTextureInfo'.
        Currently the mip mapped property of 'backendTextureInfo' is ignored. However, in the
        near future it will be required that if it is kYes then textureFulfillProc must return
        a mip mapped texture for each plane in order to successfully draw the image.
        @param gpuContextProxy     the thread-safe proxy of the gpu context. required.
        @param backendTextureInfo  info about the promised yuva gpu texture
        @param imageColorSpace     range of colors; may be nullptr
        @param textureFulfillProc  function called to get actual gpu texture
        @param textureReleaseProc  function called when texture can be deleted
        @param textureContexts     state passed to textureFulfillProc and textureReleaseProc
        @return                    created SkImage, or nullptr
    */
    static sk_sp<SkImageGanesh> MakePromiseYUVATexture(
                                                 sk_sp<GrContextThreadSafeProxy> gpuContextProxy,
                                                 const GrYUVABackendTextureInfo& backendTextureInfo,
                                                 sk_sp<SkColorSpace> imageColorSpace,
                                                 PromiseImageTextureFulfillProc textureFulfillProc,
                                                 PromiseImageTextureReleaseProc textureReleaseProc,
                                                 PromiseImageTextureContext textureContexts[]);

};
#endif
