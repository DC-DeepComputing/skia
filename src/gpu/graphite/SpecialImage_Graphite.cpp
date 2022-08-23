/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/shaders/SkImageShader.h"

namespace skgpu::graphite {

namespace {

sk_sp<SkImage> wrap_proxy_in_image(TextureProxyView view, const SkColorInfo& colorInfo) {
    return sk_make_sp<Image>(kNeedNewImageUniqueID, std::move(view), colorInfo);
}

} // anonymous namespace


class SkSpecialImage_Graphite : public SkSpecialImage_Base {
public:
    SkSpecialImage_Graphite(Recorder* recorder,
                            const SkIRect& subset,
                            uint32_t uniqueID,
                            TextureProxyView view,
                            const SkColorInfo& colorInfo,
                            const SkSurfaceProps& props)
            : SkSpecialImage_Base(subset, uniqueID, props)
            , fRecorder(recorder)
            , fTextureProxyView(std::move(view))
            , fColorInfo(colorInfo) {
    }

    SkAlphaType alphaType() const override { return fColorInfo.alphaType(); }

    SkColorType colorType() const override { return fColorInfo.colorType(); }

    size_t getSize() const override {
        return 0; // fTextureProxyView.proxy()->gpuMemorySize();
    }

    void onDraw(SkCanvas* canvas,
                SkScalar x, SkScalar y,
                const SkSamplingOptions& sampling,
                const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        // TODO: In this instance we know we're going to draw a sub-portion of the backing
        // texture into the canvas so it is okay to wrap it in an SkImage. This poses
        // some problems for full deferral however in that when the deferred SkImage_Gpu
        // instantiates itself it is going to have to either be okay with having a larger
        // than expected backing texture (unlikely) or the 'fit' of the SurfaceProxy needs
        // to be tightened (if it is deferred).
        sk_sp<SkImage> img = sk_sp<SkImage>(new skgpu::graphite::Image(this->uniqueID(),
                                                                       fTextureProxyView,
                                                                       fColorInfo));

        canvas->drawImageRect(img, SkRect::Make(this->subset()), dst,
                              sampling, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

#if SK_SUPPORT_GPU
    GrSurfaceProxyView onView1(GrRecordingContext*) const override { return {}; }
#endif

    TextureProxyView onTextureProxyView() const override { return fTextureProxyView; }

    bool onGetROPixels(SkBitmap* dst) const override {
        // This should never be called: All GPU image filters are implemented entirely on the GPU,
        // so we never perform read-back.
        SkASSERT(false);
        return false;
    }

    SkColorSpace* onGetColorSpace() const override {
        return fColorInfo.colorSpace();
    }

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType,
                                          const SkColorSpace* colorSpace,
                                          const SkISize& size,
                                          SkAlphaType at,
                                          const SkSurfaceProps& props) const override {
        SkASSERT(fRecorder);

        SkImageInfo ii = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));

        return SkSpecialSurface::MakeGraphite(fRecorder, ii, props);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeGraphite(fRecorder,
                                            subset,
                                            this->uniqueID(),
                                            fTextureProxyView,
                                            fColorInfo,
                                            this->props());
    }

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
#if 0
        TextureProxy* proxy = fTextureProxyView.proxy();

        if (subset) {
            if (proxy->isFunctionallyExact() && *subset == SkIRect::MakeSize(proxy->dimensions())) {
                proxy->priv().exactify(false);
                // The existing GrTexture is already tight so reuse it in the SkImage
                return wrap_proxy_in_image(fTextureProxyView, fColorInfo);
            }

            auto subsetView = GrSurfaceProxyView::Copy(fRecorder,
                                                       fTextureProxyView,
                                                       GrMipmapped::kNo,
                                                       *subset,
                                                       SkBackingFit::kExact,
                                                       SkBudgeted::kYes,
                    /*label=*/"SkSpecialImage_AsImage");
            if (!subsetView) {
                return nullptr;
            }
            SkASSERT(subsetView.asTextureProxy());
            SkASSERT(subsetView.proxy()->priv().isExact());

            // MDB: this is acceptable (wrapping subsetProxy in an SkImage) bc Copy will
            // return a kExact-backed proxy
            return wrap_proxy_in_image(std::move(subsetView), fColorInfo);
        }

        proxy->priv().exactify(true);
#endif

        return wrap_proxy_in_image(fTextureProxyView, fColorInfo);
    }

    sk_sp<SkShader> onAsShader(SkTileMode tileMode,
                               const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const override {
        // The special image's logical (0,0) is at its subset's topLeft() so we need to account for
        // that in the local matrix used when sampling.
        SkMatrix subsetOrigin = SkMatrix::Translate(-this->subset().topLeft());
        subsetOrigin.postConcat(lm);
        // However, we don't need to modify the subset itself since that is defined with respect to
        // the base image, and the local matrix is applied before any tiling/clamping.
        const SkRect subset = SkRect::Make(this->subset());

        // asImage() w/o a subset makes no copy; create the SkImageShader directly to remember the
        // subset used to access the image.
        return SkImageShader::MakeSubset(this->asImage(), subset, tileMode, tileMode,
                                         sampling, &subsetOrigin);
    }

    sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType,
                                        const SkColorSpace* colorSpace,
                                        const SkISize& size,
                                        SkAlphaType at) const override {
        // TODO (michaelludwig): Why does this ignore colorType but onMakeSurface doesn't ignore it?
        //    Once makeTightSurface() goes away, should this type overriding behavior be moved into
        //    onMakeSurface() or is this unnecessary?
        colorType = colorSpace && colorSpace->gammaIsLinear() ? kRGBA_F16_SkColorType
                                                              : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSurface::MakeGraphite(fRecorder, SkBudgeted::kYes, info, nullptr);
    }

private:
    Recorder*        fRecorder;
    TextureProxyView fTextureProxyView;
    SkColorInfo      fColorInfo;
};

} // namespace skgpu::graphite

namespace {

#ifdef SK_DEBUG
// TODO: maybe move into SkSpecialImage itself and share w/ other derived classes
bool rect_fits(const SkIRect& rect, int width, int height) {
    if (0 == width && 0 == height) {
        SkASSERT(0 == rect.fLeft && 0 == rect.fRight && 0 == rect.fTop && 0 == rect.fBottom);
        return true;
    }

    return rect.fLeft >= 0 && rect.fLeft < width && rect.fLeft < rect.fRight &&
           rect.fRight >= 0 && rect.fRight <= width &&
           rect.fTop >= 0 && rect.fTop < height && rect.fTop < rect.fBottom &&
           rect.fBottom >= 0 && rect.fBottom <= height;
}
#endif

} // anonymous namespace

sk_sp<SkSpecialImage> SkSpecialImage::MakeGraphite(skgpu::graphite::Recorder* recorder,
                                                   const SkIRect& subset,
                                                   uint32_t uniqueID,
                                                   skgpu::graphite::TextureProxyView view,
                                                   const SkColorInfo& colorInfo,
                                                   const SkSurfaceProps& props) {
    if (!recorder || !view.proxy()) {
        return nullptr;
    }

    SkASSERT_RELEASE(rect_fits(subset, view.width(), view.height()));
    return sk_make_sp<skgpu::graphite::SkSpecialImage_Graphite>(recorder, subset, uniqueID,
                                                                std::move(view), colorInfo, props);
}
