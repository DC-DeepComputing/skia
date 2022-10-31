/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/BackendTexture.h"

#include "include/core/SkColorFilter.h"
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

    void checkImageReleased(skiatest::Reporter* reporter, int expectedReleaseCnt) {
        REPORTER_ASSERT(reporter, expectedReleaseCnt == fImageReleaseCount);
    }

    BackendTexture fBackendTex;
    skiatest::Reporter* fReporter;
    int fFulfillCount = 0;
    int fImageReleaseCount = 0;
    int fTextureReleaseCount = 0;

    static std::tuple<BackendTexture, void*> Fulfill(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fFulfillCount++;
        return { checker->fBackendTex, self };
    }

    static void ImageRelease(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fImageReleaseCount++;
    }

    static void TextureRelease(void* self) {
        auto checker = reinterpret_cast<PromiseTextureChecker*>(self);

        checker->fTextureReleaseCount++;
    }
};

enum class ReleaseBalanceExpectation {
    kBalanced,
    kAllOffByOne,  // fulfill calls ahead of release calls by 1
    kFulfillsOnly, // 'n' fulfill calls, 0 release calls
};

void check_fulfill_and_release_cnts(skiatest::Reporter* reporter,
                                    const PromiseTextureChecker& promiseChecker,
                                    int expectedFulfillCnt,
                                    ReleaseBalanceExpectation releaseBalanceExpectation) {
    REPORTER_ASSERT(reporter, promiseChecker.fFulfillCount == expectedFulfillCnt);
    if (!expectedFulfillCnt) {
        // Release should only ever be called after Fulfill.
        REPORTER_ASSERT(reporter, !promiseChecker.fImageReleaseCount);
        REPORTER_ASSERT(reporter, !promiseChecker.fTextureReleaseCount);
        return;
    }
    int releaseDiff = promiseChecker.fFulfillCount - promiseChecker.fTextureReleaseCount;
    switch (releaseBalanceExpectation) {
        case ReleaseBalanceExpectation::kBalanced:
            SkASSERT(!releaseDiff);
            REPORTER_ASSERT(reporter, !releaseDiff);
            break;
        case ReleaseBalanceExpectation::kAllOffByOne:
            REPORTER_ASSERT(reporter, releaseDiff == 1);
            break;
        case ReleaseBalanceExpectation::kFulfillsOnly:
            REPORTER_ASSERT(reporter, releaseDiff == expectedFulfillCnt);
            break;
    }
}

void check_unfulfilled(const PromiseTextureChecker& promiseChecker,
                       skiatest::Reporter* reporter) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, /* expectedFulfillCnt= */ 0,
                                   ReleaseBalanceExpectation::kBalanced);
}

void check_fulfilled_ahead_by_one(skiatest::Reporter* reporter,
                                  const PromiseTextureChecker& promiseChecker,
                                  int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kAllOffByOne);
}

void check_all_done(skiatest::Reporter* reporter,
                    const PromiseTextureChecker& promiseChecker,
                    int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kBalanced);
}

void check_fulfills_only(skiatest::Reporter* reporter,
                         const PromiseTextureChecker& promiseChecker,
                         int expectedFulfillCnt) {
    check_fulfill_and_release_cnts(reporter, promiseChecker, expectedFulfillCnt,
                                   ReleaseBalanceExpectation::kFulfillsOnly);
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(NonVolatileGraphitePromiseImageTest,
                                         reporter,
                                         context) {
    constexpr SkISize kDimensions { 16, 16 };

    const Caps* caps = context->priv().caps();
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                                 Mipmapped::kNo,
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
                                                             PromiseTextureChecker::ImageRelease,
                                                             PromiseTextureChecker::TextureRelease,
                                                             &promiseChecker);

    sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder.get(), ii);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        check_unfulfilled(promiseChecker, reporter);

        std::unique_ptr<Recording> recording = recorder->snap();
        check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

        context->insertRecording({ recording.get() });
    }

    context->submit(SyncToCpu::kNo);
    // We still own the 'img' so we should not have called TextureRelease.
    check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

    context->submit(SyncToCpu::kYes);
    check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        canvas->drawImage(img, 0, 0);

        std::unique_ptr<Recording> recording = recorder->snap();
        // 'img' should still be fulfilled from the first time we snapped a Recording.
        check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

        context->insertRecording({ recording.get() });
    }

    context->submit(SyncToCpu::kYes);
    check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

    {
        SkCanvas* canvas = surface->getCanvas();

        canvas->drawImage(img, 0, 0);
        img.reset();

        std::unique_ptr<Recording> recording = recorder->snap();
        check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

        context->insertRecording({ recording.get() });
    }

    // img's proxy is held by the recording so, despite 'img' being freed earlier, the imageRelease
    // callback doesn't occur until the recording is deleted.
    promiseChecker.checkImageReleased(reporter, /* expectedReleaseCnt= */ 1);

    // We no longer own 'img' but the last recording is still not submitted.
    check_fulfilled_ahead_by_one(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

    context->submit(SyncToCpu::kYes);

    // Now TextureRelease should definitely have been called.
    check_all_done(reporter, promiseChecker, /* expectedFulfillCnt= */ 1);

    context->deleteBackendTexture(backendTex);
}
