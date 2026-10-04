/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkBlurEngine.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkRandom.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/graphite/ComputeProvider.h"
#include "src/gpu/graphite/ComputeScheduler.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderOptionsPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/TestOptions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>

using namespace skgpu::graphite;
using namespace skiatest::graphite;

namespace {

void enable_compute_blur(ContextOptions* options) {
    if (options->fOptionsPriv) {
        options->fOptionsPriv->fUseComputeBlur = true;
    }
}

bool is_compute_supported_context_type(skiatest::GpuContextType ctxType) {
    return skiatest::IsDawnContextType(ctxType) ||
           skiatest::IsMetalContextType(ctxType) ||
           skiatest::IsVulkanContextType(ctxType);
}

// Returns whether blurs on `context` can run with compute. Callers skip the test otherwise.
bool uses_compute_blur(Context* context) {
    return !context->supportsProtectedContent() &&
           context->priv().computeProvider() &&
           context->priv().computeProvider()->gaussianBlur();
}

std::unique_ptr<Recorder> make_recorder(Context* context, size_t batchBudget) {
    RecorderOptionsPriv optionsPriv;
    optionsPriv.fComputeBatchBudget = batchBudget;
    RecorderOptions options;
    options.fRecorderOptionsPriv = &optionsPriv;
    return context->makeRecorder(options);
}

const SkBlurEngine::Algorithm* find_algorithm(const skif::Backend* backend, SkSize sigma) {
    return backend->getBlurEngine()->findAlgorithm(sigma, backend->colorType());
}

// Uploads `bitmap`, and returns a special image of its `subset`.
sk_sp<SkSpecialImage> make_source(Recorder* recorder,
                                  const SkBitmap& bitmap,
                                  const SkIRect& subset) {
    sk_sp<SkImage> image = SkImages::TextureFromImage(recorder, bitmap.asImage().get());
    if (!image) {
        return nullptr;
    }
    return SkSpecialImages::MakeGraphite(recorder, subset, std::move(image), SkSurfaceProps());
}

// Returns a surface that `image` has been copied into, at the origin.
sk_sp<SkSurface> draw_into_surface(Recorder* recorder, const SkSpecialImage* image) {
    const SkImageInfo info = SkImageInfo::Make(
            image->dimensions(), image->colorType(), kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, info);
    if (!surface) {
        return nullptr;
    }
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    image->draw(surface->getCanvas(), 0, 0, SkSamplingOptions(), &paint);
    return surface;
}

// Reads back `surface`, which snaps and inserts the pending work of its Recorder.
SkBitmap read_back(SkSurface* surface) {
    SkBitmap bitmap;
    bitmap.allocPixels(surface->imageInfo());
    if (!surface->readPixels(bitmap, 0, 0)) {
        return {};
    }
    return bitmap;
}

const uint8_t* rgba_at(const SkBitmap& bitmap, int x, int y) {
    return reinterpret_cast<const uint8_t*>(bitmap.getAddr32(x, y));
}

// The test source: premultiplied RGBA8 noise. Blurs read the kSourceSubset of it, so that their
// rects are offset from the texture's.
constexpr SkISize kSourceSize = {45, 37};
constexpr SkIRect kSourceSubset = SkIRect::MakeXYWH(3, 2, 36, 30);

SkBitmap make_source_bitmap() {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(kSourceSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkRandom random(1234);
    for (int y = 0; y < kSourceSize.height(); ++y) {
        for (int x = 0; x < kSourceSize.width(); ++x) {
            uint8_t* rgba = reinterpret_cast<uint8_t*>(bitmap.getAddr32(x, y));
            rgba[3] = static_cast<uint8_t>(random.nextULessThan(256));
            for (int c = 0; c < 3; ++c) {
                rgba[c] = static_cast<uint8_t>(random.nextULessThan(rgba[3] + 1));
            }
        }
    }
    return bitmap;
}

struct BlurCase {
    SkSize fSigma;
    // Both are relative to kSourceSubset.
    SkIRect fSrcRect;
    SkIRect fDstRect;
};

constexpr BlurCase kBlurCases[] = {
        // A small 2D kernel, which the shader blur evaluates in a single pass.
        {{0.5f, 0.5f}, SkIRect::MakeWH(36, 30), SkIRect::MakeLTRB(-2, -2, 38, 32)},
        // The largest kernel, over several workgroups.
        {{4.f, 4.f}, SkIRect::MakeWH(36, 30), SkIRect::MakeLTRB(-12, -12, 48, 42)},
        // A horizontal blur of part of the source.
        {{1.5f, 0.f}, SkIRect::MakeLTRB(5, 4, 25, 20), SkIRect::MakeWH(36, 30)},
        // A vertical blur, into a dstRect that's disjoint from the srcRect.
        {{0.f, 3.f}, SkIRect::MakeLTRB(5, 4, 25, 20), SkIRect::MakeLTRB(28, -20, 50, 3)},
        // Different radii along each axis.
        {{2.2f, 3.7f}, SkIRect::MakeLTRB(2, 3, 33, 27), SkIRect::MakeLTRB(-7, -12, 40, 42)},
        // A srcRect that's much smaller than the kernel, so periodic tile modes wrap many times.
        {{4.f, 4.f}, SkIRect::MakeLTRB(10, 10, 11, 12), SkIRect::MakeLTRB(0, 0, 20, 20)},
};

constexpr SkTileMode kTileModes[] = {
        SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kMirror, SkTileMode::kDecal};

constexpr int kBlurCount = std::size(kBlurCases) * std::size(kTileModes);

// Returns the coordinate in [lo, lo + size) that `v` maps to with `tileMode`, or -1 if it maps to
// transparent black.
int tile_coord(int v, int lo, int size, SkTileMode tileMode) {
    auto mod = [](int a, int n) { return ((a % n) + n) % n; };
    const int d = v - lo;
    switch (tileMode) {
        case SkTileMode::kClamp:
            return lo + std::clamp(d, 0, size - 1);
        case SkTileMode::kRepeat:
            return lo + mod(d, size);
        case SkTileMode::kMirror: {
            const int m = mod(d, 2 * size);
            return lo + (m < size ? m : 2 * size - 1 - m);
        }
        case SkTileMode::kDecal:
            return d >= 0 && d < size ? v : -1;
    }
    SkUNREACHABLE;
}

// Checks `actual`, which is the blur of `source` with `blurCase` and `tileMode`, against the blur
// computed in double precision on the CPU with the same weights.
void check_against_reference(skiatest::Reporter* reporter,
                             const SkBitmap& source,
                             const BlurCase& blurCase,
                             SkTileMode tileMode,
                             const SkBitmap& actual,
                             const char* label) {
    // The step stores the horizontal sums as f16, so it can be off by one after rounding.
    constexpr int kTolerance = 1;

    const int radiusX = SkBlurEngine::SigmaToRadius(blurCase.fSigma.width());
    const int radiusY = SkBlurEngine::SigmaToRadius(blurCase.fSigma.height());
    std::array<float, SkShaderBlurAlgorithm::kMaxSamples> weightsX, weightsY;
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            blurCase.fSigma.width(), radiusX, SkSpan<float>(weightsX));
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            blurCase.fSigma.height(), radiusY, SkSpan<float>(weightsY));

    const SkIRect& srcRect = blurCase.fSrcRect;
    const SkIRect& dstRect = blurCase.fDstRect;
    for (int y = 0; y < dstRect.height(); ++y) {
        for (int x = 0; x < dstRect.width(); ++x) {
            double sum[4] = {};
            for (int ky = 0; ky <= 2 * radiusY; ++ky) {
                const int sy = tile_coord(
                        dstRect.fTop + y + ky - radiusY, srcRect.fTop, srcRect.height(), tileMode);
                for (int kx = 0; kx <= 2 * radiusX && sy >= 0; ++kx) {
                    const int sx = tile_coord(dstRect.fLeft + x + kx - radiusX,
                                              srcRect.fLeft,
                                              srcRect.width(),
                                              tileMode);
                    if (sx < 0) {
                        continue;
                    }
                    const uint8_t* texel = rgba_at(
                            source, kSourceSubset.fLeft + sx, kSourceSubset.fTop + sy);
                    const double weight = double(weightsX[kx]) * double(weightsY[ky]);
                    for (int c = 0; c < 4; ++c) {
                        sum[c] += weight * texel[c];
                    }
                }
            }
            const uint8_t* found = rgba_at(actual, x, y);
            for (int c = 0; c < 4; ++c) {
                const int expected = std::clamp(static_cast<int>(std::lround(sum[c])), 0, 255);
                if (std::abs(found[c] - expected) > kTolerance) {
                    ERRORF(reporter,
                           "%s: sigma (%g, %g), tile mode %d: channel %d at (%d, %d) is %d, "
                           "expected %d",
                           label, blurCase.fSigma.width(), blurCase.fSigma.height(),
                           static_cast<int>(tileMode), c, x, y, found[c], expected);
                    return;
                }
            }
        }
    }
}

// Runs every case in kBlurCases with every tile mode through `recorder`'s blur engine. The blurs
// are all recorded before any is read back, so on the compute blur they're batched together.
std::vector<SkBitmap> run_blur_cases(skiatest::Reporter* reporter,
                                     Recorder* recorder,
                                     const SkBitmap& source) {
    sk_sp<SkSpecialImage> src = make_source(recorder, source, kSourceSubset);
    if (!src) {
        ERRORF(reporter, "Failed to upload the source");
        return {};
    }
    sk_sp<skif::Backend> backend =
            skif::MakeGraphiteBackend(recorder, SkSurfaceProps(), kRGBA_8888_SkColorType);

    std::vector<sk_sp<SkSpecialImage>> blurs;
    for (const BlurCase& blurCase : kBlurCases) {
        for (SkTileMode tileMode : kTileModes) {
            const SkBlurEngine::Algorithm* algorithm = find_algorithm(backend.get(),
                                                                      blurCase.fSigma);
            sk_sp<SkSpecialImage> blur = algorithm->blur(
                    blurCase.fSigma, src, blurCase.fSrcRect, tileMode, blurCase.fDstRect);
            if (!blur || blur->dimensions() != blurCase.fDstRect.size()) {
                ERRORF(reporter, "Blur failed or has the wrong size");
                return {};
            }
            blurs.push_back(std::move(blur));
        }
    }

    std::vector<sk_sp<SkSurface>> surfaces;
    for (const sk_sp<SkSpecialImage>& blur : blurs) {
        surfaces.push_back(draw_into_surface(recorder, blur.get()));
    }
    std::vector<SkBitmap> results;
    for (const sk_sp<SkSurface>& surface : surfaces) {
        SkBitmap bitmap = surface ? read_back(surface.get()) : SkBitmap();
        if (bitmap.drawsNothing()) {
            ERRORF(reporter, "Failed to read back a blur");
            return {};
        }
        results.push_back(std::move(bitmap));
    }
    return results;
}

}  // namespace

// The compute blur matches a reference blur with every tile mode, including srcRects and dstRects
// that are offset, disjoint, or smaller than the kernel. It also stays close to the shader blur.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_MatchesReference,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        return;
    }
    const SkBitmap source = make_source_bitmap();

    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    std::vector<SkBitmap> computeBlurs = run_blur_cases(reporter, recorder.get(), source);
    if (computeBlurs.empty()) {
        return;
    }
    // Every blur ran with compute, in a single batch that was drained by the first read back.
    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 1,
                    "%d", scheduler->drainedBatchCount());

    // The same blurs with the shader blur, on a Context without the compute blur.
    TestOptions shaderOptions(options);
    shaderOptions.fOptionsPriv.fUseComputeBlur = false;
    ContextFactory shaderFactory(shaderOptions);
    ContextInfo shaderContext = shaderFactory.getContextInfo(testContext->contextType());
    if (!shaderContext.fContext) {
        ERRORF(reporter, "Failed to create a Context without the compute blur");
        return;
    }
    std::unique_ptr<Recorder> shaderRecorder = shaderContext.fContext->makeRecorder();
    std::vector<SkBitmap> shaderBlurs = run_blur_cases(reporter, shaderRecorder.get(), source);
    if (shaderBlurs.empty()) {
        return;
    }
    REPORTER_ASSERT(reporter,
                    shaderRecorder->priv().computeScheduler()->drainedBatchCount() == 0);

    // The shader blur has more error than the compute blur, since its two-pass blurs store the
    // horizontal pass in an RGBA8 texture.
    constexpr int kShaderTolerance = 2;
    int index = 0;
    for (const BlurCase& blurCase : kBlurCases) {
        for (SkTileMode tileMode : kTileModes) {
            const SkBitmap& computeBlur = computeBlurs[index];
            const SkBitmap& shaderBlur = shaderBlurs[index];
            ++index;
            check_against_reference(
                    reporter, source, blurCase, tileMode, computeBlur, "Compute blur");

            int maxDifference = 0;
            for (int y = 0; y < computeBlur.height(); ++y) {
                for (int x = 0; x < computeBlur.width(); ++x) {
                    for (int c = 0; c < 4; ++c) {
                        maxDifference = std::max(maxDifference,
                                                 std::abs(rgba_at(computeBlur, x, y)[c] -
                                                          rgba_at(shaderBlur, x, y)[c]));
                    }
                }
            }
            REPORTER_ASSERT(reporter, maxDifference <= kShaderTolerance,
                            "sigma (%g, %g), tile mode %d: differs from the shader blur by %d",
                            blurCase.fSigma.width(), blurCase.fSigma.height(),
                            static_cast<int>(tileMode), maxDifference);
        }
    }
    SkASSERT(index == kBlurCount);
}

// Batching doesn't change the results: sibling layers with blur filters, which batch until their
// parent is flushed, render identically when every blur is drained as soon as it's queued.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_BatchingDoesNotChangePixels,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        return;
    }
    constexpr SkColor kColors[] = {SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorMAGENTA};
    auto draw = [&](Recorder* recorder) -> SkBitmap {
        const SkImageInfo info =
                SkImageInfo::Make(128, 96, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(recorder, info);
        if (!surface) {
            return {};
        }
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorWHITE);
        for (int i = 0; i < 4; ++i) {
            sk_sp<SkImageFilter> blur =
                    SkImageFilters::Blur(1.f + i, 2.5f, SkTileMode::kDecal, nullptr);
            if (i == 3) {
                // Blurs the output of another blur.
                blur = SkImageFilters::Blur(2.f, 1.f, SkTileMode::kClamp, std::move(blur));
            }
            SkPaint layerPaint;
            layerPaint.setImageFilter(std::move(blur));
            const SkRect bounds = SkRect::MakeXYWH(4 + 31 * i, 4, 27, 88);
            canvas->saveLayer(&bounds, &layerPaint);
            SkPaint paint;
            paint.setColor(kColors[i]);
            canvas->drawCircle(bounds.centerX(), bounds.top() + 20, 9, paint);
            paint.setAlphaf(0.5f);
            canvas->drawRect(SkRect::MakeXYWH(bounds.left() + 5, bounds.top() + 50, 17, 25),
                             paint);
            canvas->restore();
        }
        return read_back(surface.get());
    };

    std::unique_ptr<Recorder> eagerRecorder = make_recorder(context, /*batchBudget=*/0);
    std::unique_ptr<Recorder> batchedRecorder = make_recorder(context, SIZE_MAX);
    const SkBitmap eager = draw(eagerRecorder.get());
    const SkBitmap batched = draw(batchedRecorder.get());
    if (eager.drawsNothing() || batched.drawsNothing()) {
        ERRORF(reporter, "Failed to draw or read back");
        return;
    }

    const int eagerBatches = eagerRecorder->priv().computeScheduler()->drainedBatchCount();
    const int batchedBatches = batchedRecorder->priv().computeScheduler()->drainedBatchCount();
    REPORTER_ASSERT(reporter, eagerBatches >= 5, "%d", eagerBatches);
    REPORTER_ASSERT(reporter, batchedBatches >= 1 && batchedBatches < eagerBatches,
                    "%d vs %d", batchedBatches, eagerBatches);

    for (int y = 0; y < eager.height(); ++y) {
        for (int x = 0; x < eager.width(); ++x) {
            if (*eager.getAddr32(x, y) != *batched.getAddr32(x, y)) {
                ERRORF(reporter, "At (%d, %d): 0x%08x when eager, 0x%08x when batched",
                       x, y, *eager.getAddr32(x, y), *batched.getAddr32(x, y));
                return;
            }
        }
    }
}

// Blurs that GaussianBlurStep doesn't support fall back to the shader blur, without queueing any
// compute work.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_FallsBackToShaderBlur,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        return;
    }
    std::unique_ptr<Recorder> recorder = context->makeRecorder();
    ComputeScheduler* scheduler = recorder->priv().computeScheduler();
    sk_sp<skif::Backend> backend =
            skif::MakeGraphiteBackend(recorder.get(), SkSurfaceProps(), kRGBA_8888_SkColorType);

    constexpr SkSize kSigma = {2.f, 2.f};
    constexpr SkIRect kSrcRect = SkIRect::MakeWH(16, 16);
    constexpr SkIRect kDstRect = SkIRect::MakeLTRB(-6, -6, 22, 22);
    const struct {
        SkColorType fColorType;
        SkAlphaType fAlphaType;
    } kUnsupported[] = {
        {kAlpha_8_SkColorType, kPremul_SkAlphaType},
        {kBGRA_8888_SkColorType, kPremul_SkAlphaType},
        {kRGBA_F16_SkColorType, kPremul_SkAlphaType},
        {kRGBA_8888_SkColorType, kUnpremul_SkAlphaType},
    };
    for (const auto& unsupported : kUnsupported) {
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::Make(
                kSrcRect.size(), unsupported.fColorType, unsupported.fAlphaType));
        bitmap.eraseColor(SkColorSetARGB(128, 64, 32, 16));
        sk_sp<SkSpecialImage> src = make_source(recorder.get(), bitmap, kSrcRect);
        if (!src) {
            continue;  // The color type isn't supported as a texture.
        }
        const SkBlurEngine::Algorithm* algorithm = find_algorithm(backend.get(), kSigma);
        sk_sp<SkSpecialImage> blur =
                algorithm->blur(kSigma, src, kSrcRect, SkTileMode::kDecal, kDstRect);
        REPORTER_ASSERT(reporter, blur && blur->dimensions() == kDstRect.size() &&
                                  blur->colorType() == unsupported.fColorType,
                        "color type %d", unsupported.fColorType);
    }

    // Identity blurs return the source as-is, like the shader blur.
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(kSrcRect.size(), kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType));
    bitmap.eraseColor(SK_ColorRED);
    sk_sp<SkSpecialImage> src = make_source(recorder.get(), bitmap, kSrcRect);
    const SkBlurEngine::Algorithm* algorithm = find_algorithm(backend.get(), {0.f, 0.f});
    REPORTER_ASSERT(reporter, src && algorithm->blur({0.f, 0.f}, src, kSrcRect,
                                                     SkTileMode::kDecal, kSrcRect) == src);

    REPORTER_ASSERT(reporter, scheduler->queuedDispatchCount() == 0);
    REPORTER_ASSERT(reporter, scheduler->drainedBatchCount() == 0);
}
