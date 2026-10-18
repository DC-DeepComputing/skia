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
#include "include/core/SkString.h"
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
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ComputeProvider.h"
#include "src/gpu/graphite/ComputeScheduler.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RecorderOptionsPriv.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/compute/ComputeTuning/ComputeTuning.h"
#include "src/gpu/graphite/compute/DownsampleStep.h"
#include "src/gpu/graphite/compute/GaussianBlurStep.h"
#include "tools/graphite/ContextFactory.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/TestOptions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
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

SkBitmap make_source_bitmap(SkISize size = kSourceSize) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(size, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkRandom random(1234);
    for (int y = 0; y < size.height(); ++y) {
        for (int x = 0; x < size.width(); ++x) {
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
    REPORTER_ASSERT(reporter, index == kBlurCount);
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

namespace {

// Returns the Context of `testContext`'s type that `factory` is set to create, which has the compute
// blur and the device's compute tuning, after `override` (if set) changes it. Returns null if the
// Context can't be created.
Context* make_tuned_context(std::unique_ptr<ContextFactory>* factory,
                            const TestOptions& options,
                            GraphiteTestContext* testContext,
                            std::function<void(ComputeTuning*)> override) {
    TestOptions tunedOptions(options);
    tunedOptions.fOptionsPriv.fUseComputeBlur = true;
    tunedOptions.fOptionsPriv.fOverrideComputeTuning = std::move(override);
    *factory = std::make_unique<ContextFactory>(tunedOptions);
    return (*factory)->getContextInfo(testContext->contextType()).fContext;
}

SkString describe(const GaussianBlurTuning& tuning) {
    return SkStringPrintf("GaussianBlurTuning{%d, %d, %d, %d, %d, %s}",
                          tuning.fTileWidthLog,
                          tuning.fTileHeightLog,
                          tuning.fVerticalSumsPerThread,
                          tuning.fHorizontalSumsPerThread,
                          tuning.fBucketStep,
                          tuning.fPadSums ? "true" : "false");
}

SkString describe(const DownsampleTuning& tuning) {
    return SkStringPrintf("DownsampleTuning{%d, %d, %d}",
                          tuning.fWorkgroupSizeLog,
                          tuning.fMinSamplesPerLaneLog,
                          tuning.fMaxClusterSizeLog);
}

// Tunings of GaussianBlurStep that cover the code paths of its SkSL: one, two and more blocks of
// rows in the vertical pass, whose invocations blur one item or loop over several, padded and
// unpadded sums, 1 to 8 sums per invocation in the horizontal pass, and every bucket step. The
// workgroup of 512 invocations exceeds the limits of some devices.
constexpr GaussianBlurTuning kBlurTunings[] = {
        // Tile logs, V, H, bucket step, pad      Tile   Blocks  Workgroup  Vertical pass
        {5, 4, 16, 4, 3, true},                // 32x16       1        128  One item
        {3, 3, 4, 1, 1, false},                // 8x8         2         64  One item
        {4, 4, 4, 1, 4, false},                // 16x16       4        256  One item
        {6, 4, 2, 2, 6, false},                // 64x16       8        512  Loop
        {7, 3, 8, 8, 12, true},                // 128x8       1        128  Loop
        {2, 1, 1, 4, 2, true},                 // 4x2         2          2  Loop
};

// Tunings of DownsampleStep that cover the code paths of its SkSL: clusters of one lane, of lanes
// that shuffle within their subgroup, and of more lanes than a subgroup has, whose first lane takes
// every sample. The workgroup of 1024 invocations exceeds the limits of some devices.
constexpr DownsampleTuning kDownsampleTunings[] = {
        // Workgroup log, min samples log, max cluster log
        DownsampleTuning{},  // The defaults.
        {7, 0, 0},           // Clusters of one lane.
        {10, 1, 2},          // Clusters of up to 4 lanes.
        {9, 3, 5},           // Clusters of up to 32 lanes, more than some subgroups have.
        {8, 0, 8},           // Clusters of up to 256 lanes, more than any subgroup has.
};

// The source of the downscale cases, which is large enough for their largest factors. Like the
// blurs, they read a subset of it.
constexpr SkISize kDownscaleSourceSize = {203, 171};
constexpr SkIRect kDownscaleSubset = SkIRect::MakeXYWH(5, 3, 196, 164);

struct DownscaleCase {
    SkISize fFactors;
    // In the downscaled image's pixels, like fDstRect.
    SkSize fSigma;
    // Relative to kDownscaleSubset, in its pixels.
    SkIRect fSrcRect;
    // Relative to the downscale of kDownscaleSubset.
    SkIRect fDstRect;
};

constexpr DownscaleCase kDownscaleCases[] = {
        // Just the downscale, since a sigma of 0 copies, of the whole subset into a dstRect that's
        // past its edges.
        {{2, 2}, {0.f, 0.f}, SkIRect::MakeWH(196, 164), SkIRect::MakeLTRB(-3, -2, 101, 85)},
        // A downscale along x only, of part of the subset.
        {{4, 1}, {0.f, 0.f}, SkIRect::MakeLTRB(6, 4, 150, 118), SkIRect::MakeLTRB(-2, -3, 42, 125)},
        // Different factors and sigmas along each axis.
        {{8, 2}, {1.5f, 2.f}, SkIRect::MakeLTRB(10, 8, 170, 140), SkIRect::MakeLTRB(-3, 0, 25, 75)},
        {{1, 32}, {3.f, 1.f}, SkIRect::MakeLTRB(0, 2, 196, 160), SkIRect::MakeLTRB(-10, -6, 205, 8)},
        // Blocks of 256 pixels, which most tunings split across the lanes of a cluster.
        {{16, 16},
         {2.5f, 2.5f},
         SkIRect::MakeLTRB(2, 6, 180, 150),
         SkIRect::MakeLTRB(-4, -4, 16, 14)},
        // The largest factor, whose blocks are larger than the srcRect, so the periodic tile modes
        // wrap within each block.
        {{64, 64}, {0.f, 0.f}, SkIRect::MakeLTRB(20, 30, 60, 62), SkIRect::MakeLTRB(-2, -2, 3, 3)},
        // A srcRect with odd edges, which the downscale can't repeat.
        {{2, 2}, {1.f, 1.f}, SkIRect::MakeLTRB(3, 5, 99, 81), SkIRect::MakeLTRB(-2, -2, 51, 43)},
};

// Returns whether downscaleAndBlur() supports `downscaleCase` with `tileMode`. The downscale
// averages pairs of pixels, which mustn't straddle the edges of the srcRect under repeat tiling,
// so the edges must be at even offsets along each axis that's downscaled.
bool supports_downscale(const DownscaleCase& downscaleCase, SkTileMode tileMode) {
    auto evenEdges = [](int factor, int lo, int hi) {
        return factor == 1 || ((lo | hi) & 1) == 0;
    };
    const SkIRect& srcRect = downscaleCase.fSrcRect;
    return tileMode != SkTileMode::kRepeat ||
           (evenEdges(downscaleCase.fFactors.width(), srcRect.fLeft, srcRect.fRight) &&
            evenEdges(downscaleCase.fFactors.height(), srcRect.fTop, srcRect.fBottom));
}

// Checks `actual`, which is downscaleAndBlur() of the kDownscaleSubset of `source` with
// `downscaleCase` and `tileMode`, against the blur computed in double precision on the CPU of the
// downscaled image, each pixel of which averages a block of source pixels with the tile mode
// applied to them.
void check_downscale_against_reference(skiatest::Reporter* reporter,
                                       const SkBitmap& source,
                                       const DownscaleCase& downscaleCase,
                                       SkTileMode tileMode,
                                       const SkBitmap& actual,
                                       const char* label) {
    const SkISize factors = downscaleCase.fFactors;
    const SkIRect& srcRect = downscaleCase.fSrcRect;
    const SkIRect& dstRect = downscaleCase.fDstRect;
    const int radiusX = SkBlurEngine::SigmaToRadius(downscaleCase.fSigma.width());
    const int radiusY = SkBlurEngine::SigmaToRadius(downscaleCase.fSigma.height());
    std::array<float, SkShaderBlurAlgorithm::kMaxSamples> weightsX, weightsY;
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            downscaleCase.fSigma.width(), radiusX, SkSpan<float>(weightsX));
    SkShaderBlurAlgorithm::Compute1DBlurKernel(
            downscaleCase.fSigma.height(), radiusY, SkSpan<float>(weightsY));
    // The downscale's output is RGBA8, whose rounding adds to the blur's.
    const int tolerance = radiusX > 0 || radiusY > 0 ? 2 : 1;

    // The downscaled pixels that the blur reads.
    const SkIRect lowRes = dstRect.makeOutset(radiusX, radiusY);
    std::vector<std::array<double, 4>> downscaled;
    downscaled.reserve(lowRes.width() * lowRes.height());
    for (int j = lowRes.fTop; j < lowRes.fBottom; ++j) {
        for (int i = lowRes.fLeft; i < lowRes.fRight; ++i) {
            std::array<double, 4> sum = {};
            for (int v = j * factors.height(); v < (j + 1) * factors.height(); ++v) {
                const int sy = tile_coord(v, srcRect.fTop, srcRect.height(), tileMode);
                for (int u = i * factors.width(); u < (i + 1) * factors.width() && sy >= 0; ++u) {
                    const int sx = tile_coord(u, srcRect.fLeft, srcRect.width(), tileMode);
                    if (sx < 0) {
                        continue;
                    }
                    const uint8_t* texel = rgba_at(
                            source, kDownscaleSubset.fLeft + sx, kDownscaleSubset.fTop + sy);
                    for (int c = 0; c < 4; ++c) {
                        sum[c] += texel[c];
                    }
                }
            }
            for (double& channel : sum) {
                channel /= factors.width() * factors.height();
            }
            downscaled.push_back(sum);
        }
    }

    for (int y = 0; y < dstRect.height(); ++y) {
        for (int x = 0; x < dstRect.width(); ++x) {
            double sum[4] = {};
            for (int ky = 0; ky <= 2 * radiusY; ++ky) {
                for (int kx = 0; kx <= 2 * radiusX; ++kx) {
                    const std::array<double, 4>& pixel =
                            downscaled[(y + ky) * lowRes.width() + x + kx];
                    const double weight = double(weightsX[kx]) * double(weightsY[ky]);
                    for (int c = 0; c < 4; ++c) {
                        sum[c] += weight * pixel[c];
                    }
                }
            }
            const uint8_t* found = rgba_at(actual, x, y);
            for (int c = 0; c < 4; ++c) {
                const int expected = std::clamp(static_cast<int>(std::lround(sum[c])), 0, 255);
                if (std::abs(found[c] - expected) > tolerance) {
                    ERRORF(reporter,
                           "%s: factors (%d, %d), sigma (%g, %g), tile mode %d: channel %d at "
                           "(%d, %d) is %d, expected %d",
                           label, factors.width(), factors.height(),
                           downscaleCase.fSigma.width(), downscaleCase.fSigma.height(),
                           static_cast<int>(tileMode), c, x, y, found[c], expected);
                    return;
                }
            }
        }
    }
}

}  // namespace

// Every tuning of GaussianBlurStep that the device supports matches the reference blur, since the
// tuning only changes how the work is split.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_TuningsMatchReference,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        INFOF(reporter, "Skipped, since the compute blur isn't supported");
        return;
    }
    const SkBitmap source = make_source_bitmap();
    for (const GaussianBlurTuning& tuning : kBlurTunings) {
        const SkString label = describe(tuning);
        std::unique_ptr<ContextFactory> factory;
        Context* tunedContext = make_tuned_context(
                &factory, options, testContext, [tuning](ComputeTuning* computeTuning) {
                    computeTuning->fGaussianBlur = tuning;
                });
        if (!tunedContext) {
            ERRORF(reporter, "%s: failed to create a Context", label.c_str());
            return;
        }
        std::string error;
        if (!GaussianBlurStep::IsSupported(tunedContext->priv().caps(), tuning, &error)) {
            INFOF(reporter, "%s: skipped, since %s", label.c_str(), error.c_str());
            continue;
        }
        if (!uses_compute_blur(tunedContext)) {
            ERRORF(reporter, "%s: the compute blur isn't enabled", label.c_str());
            continue;
        }

        std::unique_ptr<Recorder> recorder = tunedContext->makeRecorder();
        std::vector<SkBitmap> blurs = run_blur_cases(reporter, recorder.get(), source);
        if (blurs.empty()) {
            return;
        }
        REPORTER_ASSERT(reporter,
                        recorder->priv().computeScheduler()->drainedBatchCount() == 1,
                        "%s", label.c_str());
        int index = 0;
        for (const BlurCase& blurCase : kBlurCases) {
            for (SkTileMode tileMode : kTileModes) {
                check_against_reference(
                        reporter, source, blurCase, tileMode, blurs[index++], label.c_str());
            }
        }
    }
}

// downscaleAndBlur() matches a reference with every tuning of DownsampleStep that the device
// supports and every tile mode, including factors that differ per axis and blocks that are larger
// than the srcRect. It returns null for the srcRects that the downscale can't repeat.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_DownscaleMatchesReference,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        INFOF(reporter, "Skipped, since the compute blur isn't supported");
        return;
    }
    const SkBitmap source = make_source_bitmap(kDownscaleSourceSize);
    for (const DownsampleTuning& tuning : kDownsampleTunings) {
        const SkString label = describe(tuning);
        std::unique_ptr<ContextFactory> factory;
        Context* tunedContext = make_tuned_context(
                &factory, options, testContext, [tuning](ComputeTuning* computeTuning) {
                    computeTuning->fDownsample = tuning;
                });
        if (!tunedContext) {
            ERRORF(reporter, "%s: failed to create a Context", label.c_str());
            return;
        }
        std::string error;
        if (!DownsampleStep::IsSupported(tunedContext->priv().caps(), tuning, &error)) {
            INFOF(reporter, "%s: skipped, since %s", label.c_str(), error.c_str());
            continue;
        }
        if (!uses_compute_blur(tunedContext) ||
            !tunedContext->priv().computeProvider()->downsample()) {
            ERRORF(reporter, "%s: the compute downscale isn't enabled", label.c_str());
            continue;
        }

        std::unique_ptr<Recorder> recorder = tunedContext->makeRecorder();
        sk_sp<SkSpecialImage> src = make_source(recorder.get(), source, kDownscaleSubset);
        if (!src) {
            ERRORF(reporter, "Failed to upload the source");
            return;
        }
        sk_sp<skif::Backend> backend =
                skif::MakeGraphiteBackend(recorder.get(), SkSurfaceProps(), kRGBA_8888_SkColorType);

        // The downscales are all recorded before any is read back, so they're batched together.
        std::vector<sk_sp<SkSpecialImage>> results;
        for (const DownscaleCase& downscaleCase : kDownscaleCases) {
            for (SkTileMode tileMode : kTileModes) {
                const SkBlurEngine::Algorithm* algorithm =
                        find_algorithm(backend.get(), downscaleCase.fSigma);
                sk_sp<SkSpecialImage> result = algorithm->downscaleAndBlur(downscaleCase.fSigma,
                                                                           downscaleCase.fFactors,
                                                                           src,
                                                                           downscaleCase.fSrcRect,
                                                                           tileMode,
                                                                           downscaleCase.fDstRect);
                const bool supported = supports_downscale(downscaleCase, tileMode);
                if (!result != !supported ||
                    (result && result->dimensions() != downscaleCase.fDstRect.size())) {
                    ERRORF(reporter,
                           "%s: factors (%d, %d), tile mode %d: the downscale %s",
                           label.c_str(),
                           downscaleCase.fFactors.width(),
                           downscaleCase.fFactors.height(),
                           static_cast<int>(tileMode),
                           !supported ? "should have failed"
                           : result   ? "has the wrong size"
                                      : "failed");
                    return;
                }
                results.push_back(std::move(result));
            }
        }
        std::vector<sk_sp<SkSurface>> surfaces;
        for (const sk_sp<SkSpecialImage>& result : results) {
            surfaces.push_back(result ? draw_into_surface(recorder.get(), result.get()) : nullptr);
        }

        int index = 0;
        for (const DownscaleCase& downscaleCase : kDownscaleCases) {
            for (SkTileMode tileMode : kTileModes) {
                const bool hasResult = results[index] != nullptr;
                SkSurface* surface = surfaces[index].get();
                ++index;
                if (!hasResult) {
                    continue;
                }
                const SkBitmap bitmap = surface ? read_back(surface) : SkBitmap();
                if (bitmap.drawsNothing()) {
                    ERRORF(reporter, "%s: failed to read back a downscale", label.c_str());
                    return;
                }
                check_downscale_against_reference(
                        reporter, source, downscaleCase, tileMode, bitmap, label.c_str());
            }
        }
    }
}

// Tunings that are invalid, or that exceed the device's compute limits, are rejected with a reason.
// A Context with one doesn't create its step, so blurs use the shader blur.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_RejectsBadTunings,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    if (!uses_compute_blur(context)) {
        INFOF(reporter, "Skipped, since the compute blur isn't supported");
        return;
    }
    const Caps* caps = context->priv().caps();

    // The last two are valid, but exceed the workgroup invocations and storage of every device.
    constexpr GaussianBlurTuning kBadBlurTunings[] = {
            {.fTileWidthLog = 1},
            {.fTileHeightLog = 11},
            {.fVerticalSumsPerThread = 3},    // Not a power of two.
            {.fVerticalSumsPerThread = 32},   // More than the tile height.
            {.fHorizontalSumsPerThread = 2},  // Less than 4, with padded sums.
            {.fBucketStep = 5},               // Doesn't divide kMaxRadius.
            {.fTileWidthLog = 10, .fTileHeightLog = 6},              // 16384 invocations.
            {.fTileWidthLog = 10, .fHorizontalSumsPerThread = 64},   // Over 160KB of storage.
    };
    for (const GaussianBlurTuning& tuning : kBadBlurTunings) {
        std::string error;
        REPORTER_ASSERT(reporter,
                        !GaussianBlurStep::IsSupported(caps, tuning, &error) && !error.empty(),
                        "%s", describe(tuning).c_str());
    }

    constexpr DownsampleTuning kBadDownsampleTunings[] = {
            {.fWorkgroupSizeLog = 11},
            {.fMinSamplesPerLaneLog = 13},
            {.fMaxClusterSizeLog = 9},  // More than the workgroup.
            // One invocation, which is fewer than a subgroup has, so the subgroup isn't full.
            {.fWorkgroupSizeLog = 0, .fMaxClusterSizeLog = 0},
    };
    for (const DownsampleTuning& tuning : kBadDownsampleTunings) {
        std::string error;
        REPORTER_ASSERT(reporter,
                        !DownsampleStep::IsSupported(caps, tuning, &error) && !error.empty(),
                        "%s", describe(tuning).c_str());
    }

    std::unique_ptr<ContextFactory> factory;
    Context* badContext =
            make_tuned_context(&factory, options, testContext, [](ComputeTuning* computeTuning) {
                computeTuning->fGaussianBlur.fVerticalSumsPerThread = 3;
                computeTuning->fDownsample.fWorkgroupSizeLog = 11;
            });
    if (!badContext) {
        ERRORF(reporter, "Failed to create a Context");
        return;
    }
    const ComputeProvider* provider = badContext->priv().computeProvider();
    REPORTER_ASSERT(reporter, !provider || (!provider->gaussianBlur() && !provider->downsample()));
    std::unique_ptr<Recorder> recorder = badContext->makeRecorder();
    REPORTER_ASSERT(reporter, !run_blur_cases(reporter, recorder.get(), make_source_bitmap()).empty());
    REPORTER_ASSERT(reporter, recorder->priv().computeScheduler()->drainedBatchCount() == 0);
}

// The tuning that Caps selects for the device's GPU architecture is supported wherever the defaults
// are, so that the compute blur doesn't turn off on the devices that the tuning is for.
DEF_CONDITIONAL_GRAPHITE_TEST_FOR_CONTEXTS(ComputeBlur_ArchitectureTuningIsSupported,
                                           is_compute_supported_context_type,
                                           reporter,
                                           context,
                                           testContext,
                                           options,
                                           enable_compute_blur,
                                           /*condition=*/true,
                                           CtsEnforcement::kNever) {
    // Without any override of the tuning from the command line.
    std::unique_ptr<ContextFactory> factory;
    Context* archContext = make_tuned_context(&factory, options, testContext, /*override=*/nullptr);
    if (!archContext) {
        ERRORF(reporter, "Failed to create a Context");
        return;
    }
    const Caps* caps = archContext->priv().caps();
    const ComputeTuning& tuning = caps->computeTuning();
    std::string error;
    if (GaussianBlurStep::IsSupported(caps)) {
        REPORTER_ASSERT(reporter,
                        GaussianBlurStep::IsSupported(caps, tuning.fGaussianBlur, &error),
                        "%s: %s", describe(tuning.fGaussianBlur).c_str(), error.c_str());
    }
    if (DownsampleStep::IsSupported(caps)) {
        REPORTER_ASSERT(reporter,
                        DownsampleStep::IsSupported(caps, tuning.fDownsample, &error),
                        "%s: %s", describe(tuning.fDownsample).c_str(), error.c_str());
    }
}
