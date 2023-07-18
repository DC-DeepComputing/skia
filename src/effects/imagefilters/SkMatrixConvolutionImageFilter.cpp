/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

class SkMatrix;

using namespace skia_private;

namespace {

// TODO: This replicates a lot of the logic in GrMatrixConvolutionEffect::KernelWrapper. Once
// fully landed, GrMatrixConvolutionEffect will only be used for 2D Gaussian blurs, in which case
// its support for texture-backed kernels can be removed. It may also be fully removed if the 2D
// logic can be folded into GrGaussianConvolutionFragmentProcessor.


class SkMatrixConvolutionImageFilter final : public SkImageFilter_Base {
public:
    SkMatrixConvolutionImageFilter(const SkISize& kernelSize, const SkScalar* kernel,
                                   SkScalar gain, SkScalar bias, const SkIPoint& kernelOffset,
                                   bool convolveAlpha, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, /*cropRect=*/nullptr)
            , fKernel(kernel, kernelSize.width() * kernelSize.height())
            , fKernelSize(kernelSize)
            , fKernelOffset(kernelOffset)
            , fGain(gain)
            , fBias(bias)
            , fConvolveAlpha(convolveAlpha) {
        // The public factory should have ensured the kernel size would not overflow a 32-bit mul
        SkASSERT(sk_64_mul(fKernelSize.width(), fKernelSize.height()) ==
                 (fKernelSize.width() == fKernelSize.height()));
        SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
        SkASSERT(kernelOffset.fX >= 0 && kernelOffset.fX < kernelSize.fWidth);
        SkASSERT(kernelOffset.fY >= 0 && kernelOffset.fY < kernelSize.fHeight);
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMatrixConvolutionImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixConvolutionImageFilter)

    bool onAffectsTransparentBlack() const override {
        // affectsTransparentBlack() is confounded with "canComputeFastBounds" and MatrixConvolution
        // is unique in that it might not produce unbounded output, but we can'c calculate the
        // fast bounds because the kernel is applied in device space and no transform is provided
        // with that API.
        // TODO(skbug.com/14617): Accept a matrix in computeFastBounds() so that we can handle the
        // layer-space kernel case.

        // That issue aside, a matrix convolution can affect transparent black when it has a
        // non-zero bias and convolves alpha (if it doesn't convolve the alpha channel then the bias
        // applied to RGB doesn't matter for transparent black pixels).
        // NOTE: The crop image filters that wrap the matrix convolution to apply tile modes will
        // reset this property when possible.
        return true;
    }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

// FIXME maybe rename this [bounds?]sampledByKernel() and affectedByKernel()?
    // Helper functions to adjust 'bounds' by the kernel size and offset, either for what would be
    // sampled when covering 'bounds', or what could produce values when applied to 'bounds'.
    skif::LayerSpace<SkIRect> requiredInput(const skif::LayerSpace<SkIRect>& bounds) const;
    skif::LayerSpace<SkIRect> maxOutput(const skif::LayerSpace<SkIRect>& bounds) const;

    TArray<float> fKernel;

    // Unlike the majority of image filters, the kernel is applied as-is to the layer-space pixels.
    // This means that the kernel size and offset are always in the layer coordinate system.
    skif::LayerSpace<SkISize>  fKernelSize;
    skif::LayerSpace<SkIPoint> fKernelOffset;

    float fGain;
    float fBias;
    bool  fConvolveAlpha;
};

// LayerSpace doesn't have a clean type to represent 4 separate edge deltas, but the result
// is a valid layer-space rectangle, so just go back to the underlying SkIRect temporarily.
skif::LayerSpace<SkIRect> adjust(const skif::LayerSpace<SkIRect>& rect,
                                 int dl, int dt, int dr, int db) {
    SkIRect adjusted = SkIRect(rect);
    adjusted.adjust(dl, dt, dr, db);
    return skif::LayerSpace<SkIRect>(adjusted);
}

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::MatrixConvolution(const SkISize& kernelSize,
                                                       const SkScalar kernel[],
                                                       SkScalar gain,
                                                       SkScalar bias,
                                                       const SkIPoint& kernelOffset,
                                                       SkTileMode tileMode,
                                                       bool convolveAlpha,
                                                       sk_sp<SkImageFilter> input,
                                                       const CropRect& cropRect) {
    // We need to be able to read at most SK_MaxS32 bytes, so divide that
    // by the size of a scalar to know how many scalars we can read.
    static constexpr int32_t kMaxKernelSize = SK_MaxS32 / sizeof(SkScalar);
    // FIXME just impose a maximum on width and height like 2048 that should be pretty safe even
    // from a texture size POV
    if (kernelSize.width() < 1 || kernelSize.height() < 1) {
        return nullptr;
    }
    if (kMaxKernelSize / kernelSize.fWidth < kernelSize.fHeight) {
        return nullptr;
    }
    if (!kernel) {
        return nullptr;
    }
    if ((kernelOffset.fX < 0) || (kernelOffset.fX >= kernelSize.fWidth) ||
        (kernelOffset.fY < 0) || (kernelOffset.fY >= kernelSize.fHeight)) {
        return nullptr;
    }

    // The 'tileMode' behavior is not well-defined if there is no crop, so we only apply it if
    // there is a provided 'cropRect'.
    sk_sp<SkImageFilter> filter = std::move(input);
    if (cropRect && tileMode != SkTileMode::kDecal) {
        // Historically the input image was restricted to the cropRect when tiling was not kDecal
        // so that the kernel evaluated the tiled edge conditions, while a kDecal crop only affected
        // the output.
        filter = SkMakeCropImageFilter(*cropRect, tileMode, std::move(filter));
    }
    filter = sk_sp<SkImageFilter>(new SkMatrixConvolutionImageFilter(
            kernelSize, kernel, gain, bias, kernelOffset, convolveAlpha, std::move(filter)));
    if (cropRect) {
        // But regardless of the tileMode, the output is decal cropped.
        filter = SkMakeCropImageFilter(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
}

void SkRegisterMatrixConvolutionImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMatrixConvolutionImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMatrixConvolutionImageFilterImpl",
                            SkMatrixConvolutionImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkISize kernelSize;
    kernelSize.fWidth = buffer.readInt();
    kernelSize.fHeight = buffer.readInt();
    const int count = buffer.getArrayCount();

    const int64_t kernelArea = sk_64_mul(kernelSize.width(), kernelSize.height());
    if (!buffer.validate(kernelArea == count)) {
        return nullptr;
    }
    if (!buffer.validateCanReadN<SkScalar>(count)) {
        return nullptr;
    }
    AutoSTArray<16, SkScalar> kernel(count);
    if (!buffer.readScalarArray(kernel.get(), count)) {
        return nullptr;
    }
    SkScalar gain = buffer.readScalar();
    SkScalar bias = buffer.readScalar();
    SkIPoint kernelOffset;
    kernelOffset.fX = buffer.readInt();
    kernelOffset.fY = buffer.readInt();

    SkTileMode tileMode = SkTileMode::kDecal;
    if (buffer.isVersionLT(SkPicturePriv::kConvolutionImageFilterTilingUpdate)) {
        tileMode = buffer.read32LE(SkTileMode::kLastTileMode);
    } // else SkCropImageFilter handles the tile mode (if any)

    bool convolveAlpha = buffer.readBool();

    if (!buffer.isValid()) {
        return nullptr;
    }
    // NOTE: For SKPs with version >= kConvolutionImageFilterTilingUpdate, tileMode will be kDecal
    // and common.cropRect() will be null (so the factory also ignores tileMode). Any
    // cropping/tiling will have been handled by the deserialized input/outpuer Crop image filters.
    return SkImageFilters::MatrixConvolution(
                kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), common.cropRect());
}

void SkMatrixConvolutionImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeInt(fKernelSize.width());
    buffer.writeInt(fKernelSize.height());
    buffer.writeScalarArray(fKernel.data(), fKernel.size());
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeInt(fKernelOffset.x());
    buffer.writeInt(fKernelOffset.y());
    buffer.writeBool(fConvolveAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<class PixelFetcher, bool convolveAlpha>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src,
                                                  SkBitmap* result,
                                                  SkIVector& offset,
                                                  SkIRect rect,
                                                  const SkIRect& bounds) const {
    if (!rect.intersect(bounds)) {
        return;
    }
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(rect.fLeft - offset.fX, y - offset.fY);
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            SkScalar sumA = 0, sumR = 0, sumG = 0, sumB = 0;
            for (int cy = 0; cy < fKernelSize.fHeight; cy++) {
                for (int cx = 0; cx < fKernelSize.fWidth; cx++) {
                    SkPMColor s = PixelFetcher::fetch(src,
                                                      x + cx - fKernelOffset.fX,
                                                      y + cy - fKernelOffset.fY,
                                                      bounds);
                    SkScalar k = fKernel[cy * fKernelSize.fWidth + cx];
                    if (convolveAlpha) {
                        sumA += SkGetPackedA32(s) * k;
                    }
                    sumR += SkGetPackedR32(s) * k;
                    sumG += SkGetPackedG32(s) * k;
                    sumB += SkGetPackedB32(s) * k;
                }
            }
            int a = convolveAlpha
                  ? SkTPin(SkScalarFloorToInt(sumA * fGain + fBias), 0, 255)
                  : 255;
            int r = SkTPin(SkScalarFloorToInt(sumR * fGain + fBias), 0, a);
            int g = SkTPin(SkScalarFloorToInt(sumG * fGain + fBias), 0, a);
            int b = SkTPin(SkScalarFloorToInt(sumB * fGain + fBias), 0, a);
            if (!convolveAlpha) {
                a = SkGetPackedA32(PixelFetcher::fetch(src, x, y, bounds));
                *dptr++ = SkPreMultiplyARGB(a, r, g, b);
            } else {
                *dptr++ = SkPackARGB32(a, r, g, b);
            }
        }
    }
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::requiredInput(
        const skif::LayerSpace<SkIRect>& bounds) const {
    return adjust(bounds, -fKernelOffset.x(), -fKernelOffset.y(), fKernelSize.width() - fKernelOffset.x() - 1, fKernelSize.height() - fKernelOffset.y() - 1);
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::maxOutput(
        const skif::LayerSpace<SkIRect>& bounds) const {
    return adjust(bounds, fKernelOffset.x() - fKernelSize.width() + 1, fKernelOffset.y() - fKernelSize.height() + 1, fKernelOffset.x(), fKernelOffset.y());
}

skif::FilterResult SkMatrixConvolutionImageFilter::onFilterImage(
        const skif::Context& context) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(context.desiredOutput());
    skif::FilterResult childOutput = this->getChildOutput(0, context.withNewDesiredOutput(requiredInput));

    skif::LayerSpace<SkIRect> outputBounds;
    if (fConvolveAlpha && fBias != 0.f) {
        // The convolution will produce a non-trivial value for every pixel so fill desired output.
        outputBounds = context.desiredOutput();
    } else {
        // Calculate the possible extent of the convolution given what was actually produced by the
        // child filter and then intersect that with the desired output.
        outputBounds = this->maxOutput(childOutput.layerBounds());
        if (!outputBounds.intersect(context.desiredOutput())) {
            return {};
        }
    }

    // FIXME forceResolveInputs?
    skif::FilterResult::Builder builder{context};
    builder.add(childOutput, this->requiredInput(outputBounds));
    return builder.eval([this](SkSpan<sk_sp<SkShader>> inputs) {

    }, ShaderFlags::kExplicitOutputBounds | ShaderFlags::kForceResolveInputs, outputBounds);
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Adjust the desired output bounds by the kernel size to avoid evaluating edge conditions, and
    // then recurse to the child filter.
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    if (fConvolveAlpha && fBias != 0.f) {
        // Applying the kernel as a convolution to fully transparent black will result in 0 for
        // each channel, unless the bias itself shifts this "zero-point". However, when the alpha
        // channel is not convolved, the original a=0 is preserved and producing a premul color
        // discards the non-zero bias. Convolving the alpha channel and a non-zero bias can mean
        // the transparent black pixels outside of any input image become non-transparent black.
        return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
    }

    // Otherwise apply the kernel to the output bounds of the child filter.
    skif::LayerSpace<SkIRect> outputBounds =
            this->getChildOutputLayerBounds(0, mapping, contentBounds);
    return this->maxOutput(outputBounds);
}

SkRect SkMatrixConvolutionImageFilter::computeFastBounds(const SkRect& bounds) const {
    // See onAffectsTransparentBlack(), but without knowing the local-to-device transform, we don't
    // know how many pixels will be sampled by the kernel. Return unbounded to match the
    // expectations of an image filter that "affects" transparent black.
    return SkRectPriv::MakeLargeS32();
}
