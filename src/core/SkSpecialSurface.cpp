/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialSurface.h"

#include <memory>

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMallocPixelRef.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSurfacePriv.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#endif

SkSpecialSurface::SkSpecialSurface(sk_sp<SkBaseDevice> device,
                                   const SkIRect& subset)
        : fProps(device->surfaceProps().flags(), kUnknown_SkPixelGeometry)
        , fSubset(subset) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);

    fCanvas = std::make_unique<SkCanvas>(std::move(device));
    fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
    fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
}

sk_sp<SkSpecialImage> SkSpecialSurface::makeImageSnapshot() {
    sk_sp<SkSpecialImage> image(this->onMakeImageSnapshot());
    fCanvas.reset();
    return image;
}

sk_sp<SkSpecialImage> SkSpecialSurface::onMakeImageSnapshot() {
    fCanvas->restoreToCount(0);

    SkBaseDevice* baseDevice = SkCanvasPriv::TopDevice(fCanvas.get());
    if (!baseDevice) {
        return nullptr;
    }

    return baseDevice->snapSpecial(this->subset());
}

///////////////////////////////////////////////////////////////////////////////
class SkSpecialSurface_Raster : public SkSpecialSurface {
public:
    SkSpecialSurface_Raster(sk_sp<SkBaseDevice> device, SkIRect subset)
            : SkSpecialSurface(std::move(device), subset) {
    }

#if 0
protected:
    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        return SkSpecialImage::MakeFromRaster(this->subset(), fBitmap, this->props());
    }
#endif
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps& props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(info.dimensions());

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, 0);
    if (!pr) {
        return nullptr;
    }

    SkBitmap bitmap;
    bitmap.setInfo(info, info.minRowBytes());
    bitmap.setPixelRef(std::move(pr), 0, 0);

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap, props));
    if (!device) {
        return nullptr;
    }

    return sk_make_sp<SkSpecialSurface_Raster>(std::move(device), subset);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
class SkSpecialSurface_Gpu : public SkSpecialSurface {
public:
    SkSpecialSurface_Gpu(sk_sp<SkBaseDevice> device, SkIRect subset)
            : SkSpecialSurface(std::move(device), subset) {

#if 0
        fCanvas = std::make_unique<SkCanvas>(std::move(device));
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
#endif
    }

protected:
#if 0
    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fReadView.asTextureProxy()) {
            return nullptr;
        }
        GrColorType ct = SkColorTypeToGrColorType(fCanvas->imageInfo().colorType());

        // Note: SkSpecialImages can only be snapShotted once, so this call is destructive and we
        // move fReadMove.
        return SkSpecialImage::MakeDeferredFromGpu(fCanvas->recordingContext(),
                                                   this->subset(),
                                                   kNeedNewImageUniqueID_SpecialImage,
                                                   std::move(fReadView),
                                                   { ct, kPremul_SkAlphaType,
                                                     fCanvas->imageInfo().refColorSpace() },
                                                   this->props());
    }
#endif
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrRecordingContext* rContext,
                                                           const SkImageInfo& ii,
                                                           const SkSurfaceProps& props,
                                                           GrSurfaceOrigin surfaceOrigin) {
    if (!rContext) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(SkBudgeted::kYes, ii, SkBackingFit::kApprox, 1,
                                                GrMipmapped::kNo, GrProtected::kNo,
                                                surfaceOrigin, props,
                                                skgpu::v1::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SkSpecialSurface_Gpu>(std::move(device), subset);
}

#endif
