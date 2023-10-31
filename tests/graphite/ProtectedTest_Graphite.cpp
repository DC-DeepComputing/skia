/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceCache.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SharedContext.h"

#include "tests/CtsEnforcement.h"
#include "tools/gpu/ProtectedUtils.h"

//using skgpu;

static const int kSize = 8;

namespace skgpu::graphite {

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(Protected_SmokeTest_Graphite, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    if (!context->supportsProtectedContent()) {
        // Protected content not supported
        return;
    }

//    for (bool textureable : { true, false }) {
        for (Protected isProtected : { Protected::kYes, Protected::kNo }) {

            std::unique_ptr<Recorder> recorder = context->makeRecorder();

            sk_sp<SkSurface> surface = ProtectedUtils::CreateProtectedSkSurface(recorder.get(),
                                                                                { kSize, kSize },
//                                                                                textureable,
                                                                                isProtected);
            if (!surface) {
                continue;
            }

            sk_sp<SkImage> image = surface->makeImageSnapshot();
            if (!image) {
                ERRORF(reporter, "Could not makeImageSnapshot from a %s surface.",
                       isProtected == Protected::kYes ? "protected" : "unprotected");
                continue;
            }

            std::unique_ptr<Recording> recording = recorder->snap();

            context->insertRecording({ recording.get() });

            context->submit(SyncToCpu::kYes);

            REPORTER_ASSERT(reporter, Protected(image->isProtected()) == isProtected);
            ProtectedUtils::CheckImageBEProtection(image.get(), isProtected);
        }
//    }

    for (Protected isProtected : { Protected::kYes, Protected::kNo }) {
        std::unique_ptr<Recorder> recorder = context->makeRecorder();

        sk_sp<SkImage> image = ProtectedUtils::CreateProtectedSkImage(recorder.get(),
                                                                      { kSize, kSize },
                                                                      SkColors::kBlue,
                                                                      isProtected);
        if (!image) {
            continue;
        }

        std::unique_ptr<Recording> recording = recorder->snap();

        context->insertRecording({ recording.get() });

        context->submit(SyncToCpu::kYes);

        REPORTER_ASSERT(reporter, Protected(image->isProtected()) == isProtected);
        ProtectedUtils::CheckImageBEProtection(image.get(), isProtected);
    }

#if 0
    for (bool renderable : { true, false }) {
        for (bool isProtected : { true, false }) {
            GrBackendTexture beTex = dContext->createBackendTexture(16,
                                                                    16,
                                                                    kRGBA_8888_SkColorType,
                                                                    SkColors::kTransparent,
                                                                    skgpu::Mipmapped::kNo,
                                                                    GrRenderable(renderable),
                                                                    GrProtected(isProtected));

            REPORTER_ASSERT(reporter, beTex.isValid());
            REPORTER_ASSERT(reporter, beTex.isProtected() == isProtected);

            dContext->flushAndSubmit(GrSyncCpu::kYes);

            {
                sk_sp<SkImage> img = SkImages::BorrowTextureFrom(dContext, beTex,
                                                                 kTopLeft_GrSurfaceOrigin,
                                                                 kRGBA_8888_SkColorType,
                                                                 kPremul_SkAlphaType,
                        /* colorSpace= */ nullptr);

                REPORTER_ASSERT(reporter, img->isProtected() == isProtected);
            }

            if (beTex.isValid()) {
                dContext->deleteBackendTexture(beTex);
            }
        }
    }
#endif
}

} // namespace skgpu::graphite
