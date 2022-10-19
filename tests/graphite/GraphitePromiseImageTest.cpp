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

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(NonVolatileGraphitePromiseImageTest,
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
        check_only_fulfilled(reporter, promiseChecker);

        context->insertRecording({ recording.get() });
    }

    context->submit(SyncToCpu::kNo);
    // We still own the 'img' so we should not have called Release.
    check_only_fulfilled(reporter, promiseChecker);

    context->submit(SyncToCpu::kYes);
    check_only_fulfilled(reporter, promiseChecker);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        canvas->drawImage(img, 0, 0);

        std::unique_ptr<Recording> recording = recorder->snap();
        // 'img' should still be fulfilled from the first time we snapped a Recording.
        check_only_fulfilled(reporter, promiseChecker);

        context->insertRecording({ recording.get() });
    }

    context->submit(SyncToCpu::kYes);
    check_only_fulfilled(reporter, promiseChecker);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        img.reset();

        std::unique_ptr<Recording> recording = recorder->snap();
        check_only_fulfilled(reporter, promiseChecker);

        context->insertRecording({ recording.get() });
    }

    // We no longer own 'img' but the last recording is still not submitted.
    check_only_fulfilled(reporter, promiseChecker);

    context->submit(SyncToCpu::kYes);

    // Now Release should definitely have been called.
    check_all_done(reporter, promiseChecker);

    context->deleteBackendTexture(backendTex);
}
