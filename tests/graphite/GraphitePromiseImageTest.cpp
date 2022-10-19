/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tests/Test.h"

using namespace skgpu::graphite;

namespace {

struct PromiseTextureChecker {
    explicit PromiseTextureChecker(const BackendTexture& backendTex,
                                   skiatest::Reporter* reporter)
            : fBackendTex(backendTex)
            , fReporter(reporter) {
    }

    BackendTexture fBackendTex;
    skiatest::Reporter* fReporter;
    int fFulfillCount = 0;
    int fReleaseCount = 0;

    static std::tuple<BackendTexture, void*> Fulfill(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fFulfillCount++;
        return { checker->fBackendTex, self };
    }

    static void Release(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fReleaseCount++;
    }
};

enum class ReleaseBalanceExpectation {
    kBalanced,
    kAllUnbalanced,
};

void check_fulfill_and_release_cnts(skiatest::Reporter* reporter,
                                    const PromiseTextureChecker& promiseChecker,
                                    int expectedFulfillCnt,
                                    ReleaseBalanceExpectation releaseBalanceExpecation) {
    REPORTER_ASSERT(reporter, promiseChecker.fFulfillCount == expectedFulfillCnt);
    if (!expectedFulfillCnt) {
        // Release should only ever be called after Fulfill.
        REPORTER_ASSERT(reporter, !promiseChecker.fReleaseCount);
        return;
    }
    int releaseDiff = promiseChecker.fFulfillCount - promiseChecker.fReleaseCount;
    switch (releaseBalanceExpecation) {
        case ReleaseBalanceExpectation::kBalanced:
            SkASSERT(!releaseDiff);
            REPORTER_ASSERT(reporter, !releaseDiff);
            break;
        case ReleaseBalanceExpectation::kAllUnbalanced:
            REPORTER_ASSERT(reporter, releaseDiff == promiseChecker.fFulfillCount);
            break;
    }
}

void check_unfulfilled(const PromiseTextureChecker& promiseChecker,
                       skiatest::Reporter* reporter) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, /* expectedFulfillCnt= */ 0,
                                   ReleaseBalanceExpectation::kBalanced);
}

void check_only_fulfilled(skiatest::Reporter* reporter,
                          const PromiseTextureChecker& promiseChecker) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, /* expectedFulfillCnt= */ 1,
                                   ReleaseBalanceExpectation::kAllUnbalanced);
}

void check_all_done(skiatest::Reporter* reporter,
                    const PromiseTextureChecker& promiseChecker) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, /* expectedFulfillCnt= */ 1,
                                   ReleaseBalanceExpectation::kBalanced);
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(GraphitePromiseImageTest,
                                         reporter,
                                         context) {
    constexpr SkISize kDimensions { 16, 16 };

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                                 /* levelCount= */ 1,
                                                                 skgpu::Protected::kNo,
                                                                 Renderable::kYes);

    BackendTexture backendTex = recorder->createBackendTexture(kDimensions, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    PromiseTextureChecker promiseChecker(backendTex, reporter);

    SkImageInfo ii = SkImageInfo::Make(kDimensions.fWidth,
                                       kDimensions.fHeight,
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);

    sk_sp<SkImage> img = SkImage::MakeGraphitePromiseTexture(recorder.get(),
                                                             kDimensions,
                                                             textureInfo,
                                                             ii.colorInfo(),
                                                             Volatile::kNo,
                                                             PromiseTextureChecker::Fulfill,
                                                             PromiseTextureChecker::Release,
                                                             &promiseChecker);

    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder.get(), ii);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        check_unfulfilled(promiseChecker, reporter);

        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
    }

    check_only_fulfilled(reporter, promiseChecker);

    context->submit(SyncToCpu::kNo);
    // We still own the image so we should not have called Release.
    check_only_fulfilled(reporter, promiseChecker);

    context->submit(SyncToCpu::kYes);
    check_only_fulfilled(reporter, promiseChecker);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        canvas->drawImage(img, 0, 0);

        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
    }

    context->submit(SyncToCpu::kYes);

    // Image should still be fulfilled from the first time we snapped a Recording.
    check_only_fulfilled(reporter, promiseChecker);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        img.reset();

        std::unique_ptr<Recording> recording = recorder->snap();
        context->insertRecording({ recording.get() });
    }

    // We no longer own the image but the last recording is still not submitted.
    check_only_fulfilled(reporter, promiseChecker);

    context->submit(SyncToCpu::kYes);

    // Now Release should definitely have been called.
    check_all_done(reporter, promiseChecker);

    context->deleteBackendTexture(backendTex);
}

#if 0
DEF_GRAPHITE_TEST(PromiseImageTextureShutdown, reporter, ctxInfo) {
    constexpr SkISize kDimensions { 16, 16 };

    const int kWidth = 10;
    const int kHeight = 10;

    // Different ways of killing contexts.
    using DeathFn = std::function<void(sk_gpu_test::GrContextFactory*, GrDirectContext*)>;
    DeathFn destroy = [](sk_gpu_test::GrContextFactory* factory, GrDirectContext*) {
        factory->destroyContexts();
    };
    DeathFn abandon = [](sk_gpu_test::GrContextFactory* factory, GrDirectContext* dContext) {
        dContext->abandonContext();
    };
    DeathFn releaseResourcesAndAbandon = [](sk_gpu_test::GrContextFactory* factory,
                                            GrDirectContext* dContext) {
        dContext->releaseResourcesAndAbandonContext();
    };

    for (int type = 0; type < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++type) {
        auto contextType = static_cast<sk_gpu_test::GrContextFactory::ContextType>(type);
        // These tests are difficult to get working with Vulkan. See http://skbug.com/8705
        // and http://skbug.com/8275
        // Also problematic on Dawn; see http://skbug.com/10326
        // And Direct3D, for similar reasons.
        GrBackendApi api = sk_gpu_test::GrContextFactory::ContextTypeBackend(contextType);
        if (api == GrBackendApi::kVulkan || api == GrBackendApi::kDawn ||
            api == GrBackendApi::kDirect3D) {
            continue;
        }
        DeathFn contextKillers[] = {destroy, abandon, releaseResourcesAndAbandon};
        for (const DeathFn& contextDeath : contextKillers) {
            sk_gpu_test::GrContextFactory factory;
            auto ctx = factory.get(contextType);
            if (!ctx) {
                continue;
            }

            auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(ctx,
                                                                            kWidth,
                                                                            kHeight,
                                                                            kAlpha_8_SkColorType,
                                                                            GrMipmapped::kNo,
                                                                            GrRenderable::kNo);
            if (!mbet) {
                ERRORF(reporter, "Could not create texture alpha texture.");
                continue;
            }

            SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);
            sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info);
            SkCanvas* canvas = surface->getCanvas();

            PromiseTextureChecker promiseChecker(mbet->texture(), reporter);
            sk_sp<SkImage> image(SkImage_Gpu::MakePromiseTexture(ctx->threadSafeProxy(),
                                                                 mbet->texture().getBackendFormat(),
                                                                 {kWidth, kHeight},
                                                                 GrMipmapped::kNo,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 kAlpha_8_SkColorType,
                                                                 kPremul_SkAlphaType,
                                                                 /*color space*/ nullptr,
                                                                 PromiseTextureChecker::Fulfill,
                                                                 PromiseTextureChecker::Release,
                                                                 &promiseChecker));
            REPORTER_ASSERT(reporter, image);

            canvas->drawImage(image, 0, 0);
            image.reset();
            // If the surface still holds a ref to the context then the factory will not be able
            // to destroy the context (and instead will release-all-and-abandon).
            surface.reset();

            ctx->flushAndSubmit();
            contextDeath(&factory, ctx);

            check_all_done(reporter, promiseChecker);
        }
    }
}
#endif

#if 0
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(PromiseImageTextureFullCache, reporter, context) {
    constexpr SkISize kDimensions { 16, 16 };

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kAlpha_8_SkColorType,
                                                                 /* levelCount */ 1,
                                                                 skgpu::Protected::kNo,
                                                                 Renderable::kNo);
    BackendTexture backendTex = recorder->createBackendTexture(kDimensions, textureInfo);
    REPORTER_ASSERT(reporter, backendTex.isValid());

    SkImageInfo ii = SkImageInfo::Make(kDimensions.fWidth, kDimensions.fHeight,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder.get(), ii);
    SkCanvas* canvas = surface->getCanvas();

    PromiseTextureChecker promiseChecker(backendTex, reporter);
    sk_sp<SkImage> image = SkImage::MakeGraphitePromiseTexture(recorder.get(),
                                                               kDimensions,
                                                               textureInfo,
                                                               ii.colorInfo(),
                                                               Volatile::kNo,
                                                               PromiseTextureChecker::Fulfill,
                                                               PromiseTextureChecker::Release,
                                                               &promiseChecker);
    REPORTER_ASSERT(reporter, image);

    // Make the cache full. This tests that we don't preemptively purge cached textures for
    // fulfillment due to cache pressure.
    static constexpr int kMaxBytes = 1;
    dContext->setResourceCacheLimit(kMaxBytes);
    SkTArray<sk_sp<GrTexture>> textures;
    for (int i = 0; i < 5; ++i) {
        auto format = dContext->priv().caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                       GrRenderable::kNo);
        textures.emplace_back(dContext->priv().resourceProvider()->createTexture(
                {100, 100},
                format,
                GrTextureType::k2D,
                GrRenderable::kNo,
                1,
                GrMipmapped::kNo,
                SkBudgeted::kYes,
                GrProtected::kNo,
                /*label=*/"PromiseImageTextureFullCacheTest"));
        REPORTER_ASSERT(reporter, textures[i]);
    }

    size_t bytesUsed;

    dContext->getResourceCacheUsage(nullptr, &bytesUsed);
    REPORTER_ASSERT(reporter, bytesUsed > kMaxBytes);

    // Relying on the asserts in the promiseImageChecker to ensure that fulfills and releases are
    // properly ordered.
    canvas->drawImage(image, 0, 0);
    surface->flushAndSubmit();
    canvas->drawImage(image, 1, 0);
    surface->flushAndSubmit();
    canvas->drawImage(image, 2, 0);
    surface->flushAndSubmit();
    canvas->drawImage(image, 3, 0);
    surface->flushAndSubmit();
    canvas->drawImage(image, 4, 0);
    surface->flushAndSubmit();
    canvas->drawImage(image, 5, 0);
    surface->flushAndSubmit();
    // Must call these to ensure that all callbacks are performed before the checker is destroyed.
    image.reset();
    context->insertRecording({ recording });
    context->submit(SyncToCpu::kYes);

    context->deleteBackendTexture(backendTex);
}

// Test case where promise image fulfill returns nullptr.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(PromiseImageNullFulfill,
                                         reporter,
                                         context) {
    constexpr SkISize kDimensions { 16, 16 };

    const int kWidth = 10;
    const int kHeight = 10;

    auto dContext = ctxInfo.directContext();

    GrBackendFormat backendFormat =
            dContext->defaultBackendFormat(kRGBA_8888_SkColorType, GrRenderable::kYes);
    if (!backendFormat.isValid()) {
        ERRORF(reporter, "No valid default kRGBA_8888 texture format.");
        return;
    }

    struct Counts {
        int fFulfillCount = 0;
        int fReleaseCount = 0;
    } counts;
    auto fulfill = [](SkDeferredDisplayListRecorder::PromiseImageTextureContext ctx) {
        ++static_cast<Counts*>(ctx)->fFulfillCount;
        return sk_sp<SkPromiseImageTexture>();
    };
    auto release = [](SkDeferredDisplayListRecorder::PromiseImageTextureContext ctx) {
        ++static_cast<Counts*>(ctx)->fReleaseCount;
    };
    GrSurfaceOrigin texOrigin = kTopLeft_GrSurfaceOrigin;
    sk_sp<SkImage> refImg(SkImage_Gpu::MakePromiseTexture(dContext->threadSafeProxy(),
                                                          backendFormat,
                                                          {kWidth, kHeight},
                                                          GrMipmapped::kNo,
                                                          texOrigin,
                                                          kRGBA_8888_SkColorType,
                                                          kPremul_SkAlphaType,
                                                          nullptr,
                                                          fulfill,
                                                          release,
                                                          &counts));

    SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo, info);
    SkCanvas* canvas = surface->getCanvas();
    // Draw the image a few different ways.
    canvas->drawImage(refImg, 0, 0);
    SkPaint paint;
    paint.setColorFilter(SkColorFilters::LinearToSRGBGamma());
    canvas->drawImage(refImg, 0, 0, SkSamplingOptions(), &paint);
    auto shader = refImg->makeShader(SkSamplingOptions());
    REPORTER_ASSERT(reporter, shader);
    paint.setShader(std::move(shader));
    canvas->drawRect(SkRect::MakeWH(1,1), paint);
    paint.setShader(nullptr);
    refImg.reset();
    surface->flushAndSubmit();
    // We should only call each callback once and we should have made all the calls by this point.
    REPORTER_ASSERT(reporter, counts.fFulfillCount == 1);
    REPORTER_ASSERT(reporter, counts.fReleaseCount == 1);
}
#endif
