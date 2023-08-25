/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/image/SkImage_Base.h"

#include <array>

namespace {

bool valid_client_provided_image(const SkImage* clientProvided,
                                 const SkImage* original,
                                 SkImage::RequiredProperties requiredProps) {
    if (!clientProvided ||
        !as_IB(clientProvided)->isGraphiteBacked() ||
        original->dimensions() != clientProvided->dimensions() ||
        original->alphaType() != clientProvided->alphaType()) {
        return false;
    }

    uint32_t origChannels = SkColorTypeChannelFlags(original->colorType());
    uint32_t clientChannels = SkColorTypeChannelFlags(clientProvided->colorType());
    if ((origChannels & clientChannels) != origChannels) {
        return false;
    }

    // We require provided images to have a TopLeft origin
    auto graphiteImage = static_cast<const skgpu::graphite::Image*>(clientProvided);
    if (graphiteImage->textureProxyView().origin() != skgpu::Origin::kTopLeft) {
        SKGPU_LOG_E("Client provided image must have a TopLeft origin.");
        return false;
    }

    return true;
}

sk_sp<SkSpecialImage> eval_blur(skgpu::graphite::Recorder* recorder,
                                sk_sp<SkShader> blurEffect,
                                const SkIRect& dstRect,
                                sk_sp<SkColorSpace> outCS,
                                const SkSurfaceProps& outProps) {
    // FIXME we need to get color type and alpha type here somehow
    SkImageInfo outII = SkImageInfo::Make({dstRect.width(), dstRect.height()},
                                          kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                          std::move(outCS));
    auto surface = SkSpecialSurfaces::MakeGraphite(recorder, outII, outProps);
    if (!surface) {
        return nullptr;
    }

    // FIXME: This is very much like AutoSurface
    SkCanvas* canvas = surface->getCanvas();
    canvas->translate(-dstRect.left(), -dstRect.top());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(std::move(blurEffect));
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

sk_sp<SkSpecialImage> blur_2d(skgpu::graphite::Recorder* recorder,
                              SkSize sigma,
                              SkISize radii,
                              sk_sp<SkSpecialImage> input,
                              const SkIRect& srcRect,
                              const SkIRect& dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<float, skgpu::kMaxBlurSamples> kernel;
    skgpu::Compute2DBlurKernel(sigma, radii, kernel);

    SkRuntimeShaderBuilder builder{sk_ref_sp(skgpu::GetBlur2DEffect())};
    builder.uniform("kernel") = kernel;
    builder.uniform("radii") = radii;
    // FIXME: THis is very much like FilterResult::asShader()...
    builder.child("child") = input->makeSubset(srcRect)->asShader(SkTileMode::kDecal,
                                                                  SkFilterMode::kNearest,
                                                                  SkMatrix::Translate(srcRect.left(), srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect, std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur_1d(skgpu::graphite::Recorder* recorder,
                              float sigma,
                              int radius,
                              SkV2 dir,
                              sk_sp<SkSpecialImage> input,
                              SkIRect srcRect,
                              SkIRect dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<float, skgpu::kMaxBlurSamples> kernel;
    std::array<float, skgpu::kMaxBlurSamples> offsets;
    skgpu::Compute1DBlurLinearKernel(sigma, radius, kernel, offsets);

    SkRuntimeShaderBuilder builder{sk_ref_sp(skgpu::GetLinearBlur1DEffect())};
    builder.uniform("kernel") = kernel;
    builder.uniform("offsets") = offsets;
    builder.uniform("radius") = radius;
    builder.uniform("dir") = dir;
    // FIXME: THis is very much like FilterResult::asShader()...
    builder.child("child") = input->makeSubset(srcRect)->asShader(SkTileMode::kDecal,
                                                                  SkFilterMode::kLinear,
                                                                  SkMatrix::Translate(srcRect.left(), srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect, std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur(skgpu::graphite::Recorder* recorder,
                           SkSize sigma,
                           sk_sp<SkSpecialImage> input,
                           SkIRect srcRect,
                           SkIRect dstRect,
                           sk_sp<SkColorSpace> outCS,
                           const SkSurfaceProps& outProps) {
    // See if we can do a blur on the original resolution image
    if (sigma.width() <= skgpu::kMaxLinearBlurSigma &&
        sigma.height() <= skgpu::kMaxLinearBlurSigma) {
        int radiusX = skgpu::BlurSigmaRadius(sigma.width());
        int radiusY = skgpu::BlurSigmaRadius(sigma.height());
        const int kernelArea = skgpu::BlurKernelWidth(radiusX) * skgpu::BlurKernelWidth(radiusY);
        if (kernelArea <= skgpu::kMaxBlurSamples && radiusX > 0 && radiusY > 0) {
            // Use a single-pass 2D kernel if it fits and isn't just 1D already
            return blur_2d(recorder, sigma, {radiusX, radiusY}, std::move(input), srcRect, dstRect,
                           std::move(outCS), outProps);
        } else {
            // Use two passes of a 1D kernel (one per axis).
            if (radiusX > 0) {
                SkIRect intermediateDstRect = dstRect;
                if (radiusY > 0) {
                    // Outset the output size of 'dstRect' by the radius required for the next Y pass
                    intermediateDstRect.outset(0, radiusY);
                    if (!intermediateDstRect.intersect(srcRect.makeOutset(radiusX, radiusY))) {
                        return nullptr;
                    }
                }

                input = blur_1d(recorder, sigma.width(), radiusX, {1.f, 0.f}, std::move(input), srcRect, intermediateDstRect, outCS, outProps);
                if (!input) {
                    return nullptr;
                }
                srcRect = SkIRect::MakeWH(input->width(), input->height());
                dstRect.offset(-intermediateDstRect.left(), -intermediateDstRect.top());
            }

            if (radiusY > 0) {
                input = blur_1d(recorder, sigma.height(), radiusY, {0.f, 1.f}, std::move(input), srcRect, dstRect, outCS, outProps);
            }

            return input;
        }
    } else {
        // Rescale the source image, blur that with a reduced sigma, and then upscale back to the
        // dstRect dimensions.
        // TODO: Share rescaling logic with GrBlurUtils::GaussianBlur.
        float sx = sigma.width() > skgpu::kMaxLinearBlurSigma
                ? (skgpu::kMaxLinearBlurSigma / sigma.width()) : 1.f;
        float sy = sigma.height() > skgpu::kMaxLinearBlurSigma
                ? (skgpu::kMaxLinearBlurSigma / sigma.height()) : 1.f;

        int targetSrcWidth = sk_float_ceil2int(srcRect.width() * sx);
        int targetSrcHeight = sk_float_ceil2int(srcRect.height() * sy);

        auto inputImage = input->asImage();
        // TODO: support approx fit backings for the target of a rescale
        // TODO: Be smarter about downscaling when there are actual tilemodes to apply to the image.
        auto scaledInput = skgpu::graphite::RescaleImage(
                recorder,
                inputImage.get(),
                srcRect,
                inputImage->imageInfo().makeWH(targetSrcWidth, targetSrcHeight),
                SkImage::RescaleGamma::kLinear,
                SkImage::RescaleMode::kRepeatedLinear);
        if (!scaledInput) {
            return nullptr;
        }

        // Calculate a scaled dstRect to match (0,0,targetSrcWidth,targetSrcHeight) as srcRect.
        SkIRect targetDstRect = SkRect::MakeXYWH((dstRect.left() - srcRect.left()) * sx,
                                                 (dstRect.top() - srcRect.top()) * sy,
                                                 dstRect.width()*sx,
                                                 dstRect.height()*sy).roundOut();
        SkIRect targetSrcRect = SkIRect::MakeWH(targetSrcWidth, targetSrcHeight);
        // Blur with pinned sigmas. If the sigma was less than the max, that axis of the image was
        // not scaled so we can use the original. If it was greater than the max, the scale factor
        // should have taken it the max supported sigma (ignoring the effect of rounding out the
        // source bounds).
        auto scaledOutput = blur(
                recorder,
                {std::min(sigma.width(), skgpu::kMaxLinearBlurSigma),
                 std::min(sigma.height(), skgpu::kMaxLinearBlurSigma)},
                SkSpecialImages::MakeGraphite(recorder, targetSrcRect, std::move(scaledInput), outProps),
                targetSrcRect,
                targetDstRect,
                outCS,
                outProps);
        if (!scaledOutput) {
            return nullptr;
        }

        // TODO: Pass out the upscaling transform for skif::FilterResult to hold on to.
        auto scaledOutputImage = scaledOutput->asImage();
        auto outputImage = skgpu::graphite::RescaleImage(
                recorder,
                scaledOutputImage.get(),
                scaledOutputImage->bounds(),
                scaledOutputImage->imageInfo().makeWH(dstRect.width(), dstRect.height()),
                SkImage::RescaleGamma::kLinear,
                SkImage::RescaleMode::kLinear);
        if (!outputImage) {
            return nullptr;
        }

        SkIRect outputDstRect = outputImage->bounds(); // TODO: approx fit would change this
        return SkSpecialImages::MakeGraphite(recorder, outputDstRect, std::move(outputImage), outProps);
    }
}

} // anonymous namespace

namespace skgpu::graphite {

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    skgpu::Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                     ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    if (imageIn->dimensions().area() <= 1 && mipmapped == skgpu::Mipmapped::kYes) {
        mipmapped = skgpu::Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    sk_sp<SkImage> result;
    if (as_IB(imageIn)->isGraphiteBacked()) {
        result = sk_ref_sp(imageIn);

        // If the preexisting Graphite-backed image doesn't have the required mipmaps we will drop
        // down the sampling
        if (mipmapped == skgpu::Mipmapped::kYes && !result->hasMipmaps()) {
            mipmapped = skgpu::Mipmapped::kNo;
            sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        }
    } else {
        auto clientImageProvider = recorder->clientImageProvider();
        result = clientImageProvider->findOrCreate(
                recorder, imageIn, {mipmapped == skgpu::Mipmapped::kYes});

        if (!valid_client_provided_image(
                    result.get(), imageIn, {mipmapped == skgpu::Mipmapped::kYes})) {
            // The client did not fulfill the ImageProvider contract so drop the image.
            result = nullptr;
        }
    }

    if (sampling.isAniso() && result) {
        sampling = SkSamplingPriv::AnisoFallback(result->hasMipmaps());
    }

    return { result, sampling };
}

std::tuple<skgpu::graphite::TextureProxyView, SkColorType> AsView(Recorder* recorder,
                                                                  const SkImage* image,
                                                                  skgpu::Mipmapped mipmapped) {
    if (!recorder || !image) {
        return {};
    }

    if (!as_IB(image)->isGraphiteBacked()) {
        return {};
    }
    // TODO(b/238756380): YUVA not supported yet
    if (as_IB(image)->isYUVA()) {
        return {};
    }

    auto gi = reinterpret_cast<const skgpu::graphite::Image*>(image);

    if (gi->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        gi->textureProxyView().proxy()->mipmapped() != skgpu::Mipmapped::kYes) {
        SKGPU_LOG_W("Graphite does not auto-generate mipmap levels");
        return {};
    }

    SkColorType ct = gi->colorType();
    return {gi->textureProxyView(), ct};
}

} // namespace skgpu::graphite

namespace skif {

Context MakeGraphiteContext(skgpu::graphite::Recorder* recorder,
                            const ContextInfo& info) {
    SkASSERT(recorder);
    SkASSERT(!info.fSource.image() || info.fSource.image()->isGraphiteBacked());

    auto makeSurfaceFunctor = [recorder](const SkImageInfo& imageInfo,
                                         const SkSurfaceProps* props) {
        return SkSpecialSurfaces::MakeGraphite(recorder, imageInfo, *props);
    };
    auto makeImageFunctor = [recorder](const SkIRect& subset,
                                sk_sp<SkImage> image,
                                const SkSurfaceProps& props) {
        // This just makes a raster image, but it could maybe call MakeFromGraphite
        return SkSpecialImages::MakeGraphite(recorder, subset, image, props);
    };
    auto makeCachedBitmapFunctor = [recorder](const SkBitmap& data) -> sk_sp<SkImage> {
        auto proxy = skgpu::graphite::RecorderPriv::CreateCachedProxy(recorder, data);
        if (!proxy) {
            return nullptr;
        }

        const SkColorInfo& colorInfo = data.info().colorInfo();
        skgpu::Swizzle swizzle = recorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                                         proxy->textureInfo());
        return sk_make_sp<skgpu::graphite::Image>(
                data.getGenerationID(),
                skgpu::graphite::TextureProxyView(std::move(proxy), swizzle),
                colorInfo);
    };
    auto blurImageFunctor = [recorder](SkSize sigma,
                                       sk_sp<SkSpecialImage> input,
                                       SkIRect srcRect,
                                       SkIRect dstRect,
                                       sk_sp<SkColorSpace> outCS,
                                       const SkSurfaceProps& outProps) {
        return blur(recorder, sigma, std::move(input), srcRect, dstRect,
                    std::move(outCS), outProps);
    };

    return Context(info,
                   makeSurfaceFunctor,
                   makeImageFunctor,
                   makeCachedBitmapFunctor,
                   blurImageFunctor);
}
}  // namespace skif
