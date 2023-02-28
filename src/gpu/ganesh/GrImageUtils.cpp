/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrImageUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkMath.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/gpu/ganesh/GrImagePin.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"

namespace skgpu::ganesh {

static GrSurfaceProxyView copy_view(GrRecordingContext* context,
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

static std::tuple<GrSurfaceProxyView, GrColorType>
        raster_as_view(GrRecordingContext* rContext, const SkImage_Raster* raster,
                       GrMipmapped mipmapped, GrImageTexGenPolicy policy) {

    if (PinnedData* data = raster->pinnedData(); data != nullptr) {
        // We ignore the mipmap request here. If the pinned view isn't mipmapped then we will
        // fallback to bilinear. The pin API is used by Android Framework which does not expose
        // mipmapping . Moreover, we're moving towards requiring that images be made with mip levels
        // if mipmapping is desired (skbug.com/10411)
        mipmapped = GrMipmapped::kNo;
        if (policy != GrImageTexGenPolicy::kDraw) {
            return {copy_view(rContext,
                              data->fPinnedView,
                              mipmapped,
                              policy,
                              /*label=*/"TextureForImageRasterWithPolicyNotEqualKDraw"),
                    data->fPinnedColorType};
        }
        return {data->fPinnedView, data->fPinnedColorType};
    }
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
    return GrMakeUncachedBitmapProxyView(rContext,
                                         raster->bitmap(),
                                         mipmapped,
                                         SkBackingFit::kExact,
                                         budgeted);
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
    switch (ib->type()) {
        case ImageType::kRaster:
            return raster_as_view(rContext, static_cast<const SkImage_Raster*>(ib),
                                  mipmapped, policy);
        default:
            return ib->onAsView(rContext, mipmapped, policy);
    }
    SkUNREACHABLE;
}

/**
 * Returns a GrFragmentProcessor that can be used with the passed GrRecordingContext to
 * draw the image. SkSamplingOptions indicates the filter and SkTileMode[] indicates the x and
 * y tile modes. The passed matrix is applied to the coordinates before sampling the image.
 * Optional 'subset' indicates whether the tile modes should be applied to a subset of the image
 * Optional 'domain' is a bound on the coordinates of the image that will be required and can be
 * used to optimize the shader if 'subset' is also specified.
 */
std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext* rContext,
                                                         sk_sp<const SkImage> img,
                                                         SkSamplingOptions,
                                                         const SkTileMode tm[2],
                                                         const SkMatrix& m,
                                                         const SkRect* subset,
                                                         const SkRect* domain) {
    return nullptr;
};

} // skgpu::ganesh
