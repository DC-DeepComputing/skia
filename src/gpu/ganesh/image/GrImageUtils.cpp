/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/GrImageUtils.h"

#include "include/core/SkImage.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/GrTextureGenerator.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"
#include "src/image/SkImage_Lazy.h"
#include "src/gpu/ganesh/image/SkImage_RasterPinnable.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrYUVtoRGBEffect.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkBitmapCache.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkNextID.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkYUVPlanesCache.h"

#include <string_view>
#include <utility>

class SkMatrix;
enum SkAlphaType : int;
enum class GrColorType;
enum class SkTileMode;
struct SkRect;

namespace skgpu::ganesh {

GrSurfaceProxyView CopyView(GrRecordingContext* context,
                            GrSurfaceProxyView src,
                            GrMipmapped mipmapped,
                            GrImageTexGenPolicy policy,
                            std::string_view label) {
    skgpu::Budgeted budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                       ? skgpu::Budgeted::kYes
                                       : skgpu::Budgeted::kNo;
    return GrSurfaceProxyView::Copy(context,
                                    std::move(src),
                                    mipmapped,
                                    SkBackingFit::kExact,
                                    budgeted,
                                    /*label=*/label);
}

std::tuple<GrSurfaceProxyView, GrColorType> RasterAsView(GrRecordingContext* rContext,
                                                         const SkImage_Raster* raster,
                                                         GrMipmapped mipmapped,
                                                         GrImageTexGenPolicy policy) {
    if (policy == GrImageTexGenPolicy::kDraw) {
        // If the draw doesn't require mipmaps but this SkImage has them go ahead and make a
        // mipmapped texture. There are three reasons for this:
        // 1) Avoiding another texture creation if a later draw requires mipmaps.
        // 2) Ensuring we upload the bitmap's levels instead of generating on the GPU from the base.
        if (raster->hasMipmaps()) {
            mipmapped = GrMipmapped::kYes;
        }
        return GrMakeCachedBitmapProxyView(rContext,
                                           raster->bitmap(),
                                           /*label=*/"TextureForImageRasterWithPolicyEqualKDraw",
                                           mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
                            ? skgpu::Budgeted::kNo
                            : skgpu::Budgeted::kYes;
    return GrMakeUncachedBitmapProxyView(
            rContext, raster->bitmap(), mipmapped, SkBackingFit::kExact, budgeted);
}

// Returns the GrColorType to use with the GrTextureProxy returned from lockTextureProxy. This
// may be different from the color type on the image in the case where we need up upload CPU
// data to a texture but the GPU doesn't support the format of CPU data. In this case we convert
// the data to RGBA_8888 unorm on the CPU then upload that.
static GrColorType color_type_of_lock_texture_proxy(const GrCaps* caps, SkColorType sct) {
    GrColorType ct = SkColorTypeToGrColorType(sct);
    GrBackendFormat format = caps->getDefaultBackendFormat(ct, GrRenderable::kNo);
    if (!format.isValid()) {
        ct = GrColorType::kRGBA_8888;
    }
    return ct;
}

static GrSurfaceOrigin get_origin(const SkImage_Lazy* img) {
    SkASSERT(img->generator());
    if (!img->generator()->isTextureGenerator()) {
        return kTopLeft_GrSurfaceOrigin;
    }
    auto tg = static_cast<const GrTextureGenerator*>(img->generator().get());
    return tg->origin();
}

// Returns the texture proxy. We will always cache the generated texture on success.
// We have 4 ways to try to return a texture (in sorted order)
//
// 1. Check the cache for a pre-existing one
// 2. Ask the generator to natively create one
// 3. Ask the generator to return YUV planes, which the GPU can convert
// 4. Ask the generator to return RGB(A) data, which the GPU can convert
static GrSurfaceProxyView lock_texture_proxy_view(GrRecordingContext* rContext,
                                                  const SkImage_Lazy* img,
                                                  GrImageTexGenPolicy texGenPolicy,
                                                  GrMipmapped mipmapped) {
    // Values representing the various texture lock paths we can take. Used for logging the path
    // taken to a histogram.
    enum LockTexturePath {
        kFailure_LockTexturePath,
        kPreExisting_LockTexturePath,
        kNative_LockTexturePath,
        kCompressed_LockTexturePath, // Deprecated
        kYUV_LockTexturePath,
        kRGBA_LockTexturePath,
    };

    enum { kLockTexturePathCount = kRGBA_LockTexturePath + 1 };

    skgpu::UniqueKey key;
    if (texGenPolicy == GrImageTexGenPolicy::kDraw) {
        GrMakeKeyFromImageID(&key, img->uniqueID(), SkIRect::MakeSize(img->dimensions()));
    }

    const GrCaps* caps = rContext->priv().caps();
    GrProxyProvider* proxyProvider = rContext->priv().proxyProvider();

    auto installKey = [&](const GrSurfaceProxyView& view) {
        SkASSERT(view && view.asTextureProxy());
        if (key.isValid()) {
            auto listener = GrMakeUniqueKeyInvalidationListener(&key, rContext->priv().contextID());
            img->addUniqueIDListener(std::move(listener));
            proxyProvider->assignUniqueKeyToProxy(key, view.asTextureProxy());
        }
    };

    auto ct = color_type_of_lock_texture_proxy(caps, img->colorType());

    // 1. Check the cache for a pre-existing one.
    if (key.isValid()) {
        auto proxy = proxyProvider->findOrCreateProxyByUniqueKey(key);
        if (proxy) {
            skgpu::Swizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
            GrSurfaceOrigin origin = get_origin(;
            GrSurfaceProxyView view(std::move(proxy), origin, swizzle);
            if (mipmapped == GrMipmapped::kNo ||
                view.asTextureProxy()->mipmapped() == GrMipmapped::kYes) {
                return view;
            } else {
                // We need a mipped proxy, but we found a cached proxy that wasn't mipped. Thus we
                // generate a new mipped surface and copy the original proxy into the base layer. We
                // will then let the gpu generate the rest of the mips.
                auto mippedView = GrCopyBaseMipMapToView(rContext, view);
                if (!mippedView) {
                    // We failed to make a mipped proxy with the base copied into it. This could
                    // have been from failure to make the proxy or failure to do the copy. Thus we
                    // will fall back to just using the non mipped proxy; See skbug.com/7094.
                    return view;
                }
                proxyProvider->removeUniqueKeyFromProxy(view.asTextureProxy());
                installKey(mippedView);
                return mippedView;
            }
        }
    }

    // 2. Ask the generator to natively create one (if it knows how)
    {
        auto gen = img->generator();
        if (gen->isTextureGenerator()) {
            SkAutoMutexExclusive acquired(img->generator()->fMutex);
            auto generator = static_cast<const GrTextureGenerator*>(img->generator());
            if (auto view = generator->generateTexture(rContext,
                                                       img->imageInfo(),
                                                       mipmapped,
                                                       texGenPolicy)) {
                installKey(view);
                return view;
            }
        }
    }

    // 3. Ask the generator to return YUV planes, which the GPU can convert. If we will be mipping
    //    the texture we skip this step so the CPU generate non-planar MIP maps for us.
    if (mipmapped == GrMipmapped::kNo && !rContext->priv().options().fDisableGpuYUVConversion) {
        // TODO: Update to create the mipped surface in the textureProxyViewFromPlanes generator and
        //  draw the base layer directly into the mipped surface.
        skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                           ? skgpu::Budgeted::kNo
                                           : skgpu::Budgeted::kYes;
        auto view = texture_proxy_view_from_planes(rContext, img, budgeted);
        if (view) {
            installKey(view);
            return view;
        }
    }

    // 4. Ask the generator to return a bitmap, which the GPU can convert.
    auto hint = texGenPolicy == GrImageTexGenPolicy::kDraw ? CachingHint::kAllow_CachingHint
                                                           : CachingHint::kDisallow_CachingHint;
    if (SkBitmap bitmap; img->getROPixels(nullptr, &bitmap, hint)) {
        // We always make an uncached bitmap here because we will cache it based on passed in policy
        // with *our* key, not a key derived from bitmap. We're just making the proxy here.
        auto budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                ? skgpu::Budgeted::kNo
                                : skgpu::Budgeted::kYes;
        auto view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext,
                                                              bitmap,
                                                              mipmapped,
                                                              SkBackingFit::kExact,
                                                              budgeted));
        if (view) {
            installKey(view);
            return view;
        }
    }

    return {};
}

static std::tuple<GrSurfaceProxyView, GrColorType> lazy_as_view(GrRecordingContext* context,
                                                                const SkImage_Lazy* img,
                                                                GrMipmapped mipmapped,
                                                                GrImageTexGenPolicy policy) {
    GrColorType ct = color_type_of_lock_texture_proxy(context->priv().caps(), img->colorType());
    return {lock_texture_proxy_view(context, img, policy, mipmapped), ct};
}

std::tuple<GrSurfaceProxyView, GrColorType> AsView(GrRecordingContext* rContext,
                                                   const SkImage* img,
                                                   GrMipmapped mipmapped,
                                                   GrImageTexGenPolicy policy) {
    SkASSERT(img);
    if (!rContext) {
        return {};
    }
    if (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1) {
        mipmapped = GrMipmapped::kNo;
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    if (ib->type() == SkImage_Base::Type::kRaster) {
        return skgpu::ganesh::RasterAsView(
                    rContext, static_cast<const SkImage_Raster*>(ib), mipmapped, policy);
    } else if (ib->type() == SkImage_Base::Type::kRasterPinnable) {
        auto rp = static_cast<const SkImage_RasterPinnable*>(img);
        return rp->asView(rContext, mipmapped, policy);
    } else if (ib->isGaneshBacked()) {
        auto gb = static_cast<const SkImage_GaneshBase*>(img);
        return gb->asView(rContext, mipmapped, policy);
    } else if (ib->isLazyGenerated()) {
        return lazy_as_view(rContext, static_cast<const SkImage_Lazy*>(ib), mipmapped, policy)
    }

    SkDEBUGFAIL("Unsupported image type to make a View");
    return {};
}

static std::unique_ptr<GrFragmentProcessor> make_fp_from_view(GrRecordingContext* rContext,
                                                              GrSurfaceProxyView view,
                                                              SkAlphaType at,
                                                              SkSamplingOptions sampling,
                                                              const SkTileMode tileModes[2],
                                                              const SkMatrix& m,
                                                              const SkRect* subset,
                                                              const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (sampling.isAniso()) {
        if (!rContext->priv().caps()->anisoSupport()) {
            // Fallback to linear
            sampling = SkSamplingPriv::AnisoFallback(view.mipmapped() == GrMipmapped::kYes);
        }
    } else if (view.mipmapped() == GrMipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler;
    if (sampling.isAniso()) {
        sampler = GrSamplerState::Aniso(wmx, wmy, sampling.maxAniso, view.mipmapped());
    } else {
        sampler = GrSamplerState(wmx, wmy, sampling.filter, sampling.mipmap);
    }
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(
                    std::move(view), at, m, sampler, *subset, *domain, caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view), at, m, sampler, *subset, caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

std::unique_ptr<GrFragmentProcessor> raster_as_fp(GrRecordingContext* rContext,
                                                  const SkImage_Raster* img,
                                                  SkSamplingOptions sampling,
                                                  const SkTileMode tileModes[2],
                                                  const SkMatrix& m,
                                                  const SkRect* subset,
                                                  const SkRect* domain) {
    auto mm = sampling.mipmap == SkMipmapMode::kNone ? GrMipmapped::kNo : GrMipmapped::kYes;
    return make_fp_from_view(rContext,
                             std::get<0>(AsView(rContext, img, mm)),
                             img->alphaType(),
                             sampling,
                             tileModes,
                             m,
                             subset,
                             domain);
}

std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext* rContext,
                                                         const SkImage* img,
                                                         SkSamplingOptions sampling,
                                                         const SkTileMode tileModes[2],
                                                         const SkMatrix& m,
                                                         const SkRect* subset,
                                                         const SkRect* domain) {
    if (!rContext) {
        return {};
    }
    if (sampling.useCubic && !GrValidCubicResampler(sampling.cubic)) {
        return {};
    }
    if (sampling.mipmap != SkMipmapMode::kNone &&
        (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1)) {
        sampling = SkSamplingOptions(sampling.filter);
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    if (ib->isRasterBacked()) {
        return raster_as_fp(rContext,
                            static_cast<const SkImage_Raster*>(ib),
                            sampling,
                            tileModes,
                            m,
                            subset,
                            domain);
    } else if (ib->isGaneshBacked()) {
        auto gb = static_cast<const SkImage_GaneshBase*>(img);
        return gb->asFragmentProcessor(rContext, sampling, tileModes, m, subset, domain);
    } else if (ib->isLazyGenerated()) {
        // return lazy_as_fp();
    }

    SkDEBUGFAIL("Unsupported image type to make a FragmentProcessor");
    return {};
}

}  // namespace skgpu::ganesh
