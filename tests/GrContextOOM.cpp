/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/ContextType.h"

#if defined(SK_GL)
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <array>
#endif

DEF_GANESH_TEST(GrContext_oomed, reporter, originalOptions, CtsEnforcement::kApiLevel_T) {
    GrContextOptions options = originalOptions;
    options.fRandomGLOOM = true;
    options.fSkipGLErrorChecks = GrContextOptions::Enable::kNo;
    sk_gpu_test::GrContextFactory factory(options);
    for (int ct = 0; ct < skgpu::kContextTypeCount; ++ct) {
        auto contextType = static_cast<skgpu::ContextType>(ct);
        auto context = factory.get(contextType);
        if (!context) {
            continue;
        }
        if (context->backend() != GrBackendApi::kOpenGL) {
            continue;
        }
        auto info = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        for (int run = 0; run < 20; ++run) {
            bool oomed = false;
            for (int i = 0; i < 500; ++i) {
                // Make sure we're actually making a significant number of GL calls and not just
                // issuing a small number calls by reusing scratch resources created in a previous
                // iteration.
                context->freeGpuResources();
                auto surf =
                        SkSurfaces::RenderTarget(context, skgpu::Budgeted::kYes, info, 1, nullptr);
                SkPaint paint;
                surf->getCanvas()->drawRect(SkRect::MakeLTRB(100, 100, 2000, 2000), paint);
                context->flushAndSubmit(surf.get(), GrSyncCpu::kNo);
                if ((oomed = context->oomed())) {
                    REPORTER_ASSERT(reporter, !context->oomed(), "oomed() wasn't cleared");
                    break;
                }
            }
            if (!oomed) {
                ERRORF(reporter, "Did not OOM on %dth run.", run);
            }
        }
    }
}

#if defined(SK_GL)
// Verifies that GrDirectContext::checkAndClearBackendErrors() drains client-generated GL errors,
// latches OOMs so oomed() reports them, and fails an active verify ops scope.
DEF_GANESH_TEST_FOR_GL_CONTEXT(GrContext_checkAndClearBackendErrors,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kNextRelease) {
    // Errors queued here are returned by glGetError() before any real driver errors. This must
    // outlive the GrDirectContext below, which captures it by pointer.
    struct InjectedErrors {
        GrGLFunction<GrGLGetErrorFn> fGetError;
        std::array<GrGLenum, 4> fErrors = {};
        int fCount = 0;

        void push(GrGLenum error) {
            SkASSERT_RELEASE(fCount < static_cast<int>(fErrors.size()));
            fErrors[fCount++] = error;
        }
    } injected;

    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    glCtx->makeCurrent();
    auto glInterface = sk_make_sp<GrGLInterface>(*glCtx->gl());
    injected.fGetError = glInterface->fFunctions.fGetError;
    InjectedErrors* injectedPtr = &injected;
    glInterface->fFunctions.fGetError = [injectedPtr]() -> GrGLenum {
        if (injectedPtr->fCount > 0) {
            GrGLenum error = injectedPtr->fErrors[0];
            for (int i = 1; i < injectedPtr->fCount; ++i) {
                injectedPtr->fErrors[i - 1] = injectedPtr->fErrors[i];
            }
            --injectedPtr->fCount;
            return error;
        }
        return injectedPtr->fGetError();
    };
#if GR_GL_CHECK_ERROR
    // Suppress logging GL errors since we'll be synthetically generating them.
    glInterface->suppressErrorLogging();
#endif

    {
        sk_sp<GrDirectContext> context =
                GrDirectContexts::MakeGL(glInterface, GrContextOptions());
        if (!context) {
            ERRORF(reporter, "Could not create GrDirectContext.");
            return;
        }

        // Start from a clean slate.
        context->checkAndClearBackendErrors();
        context->oomed();
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());

        // A real error generated by a client GL call on Skia's context is drained.
        glInterface->fFunctions.fActiveTexture(0);  // GL_INVALID_ENUM
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, !context->oomed());

        // Multiple errors are all drained, and an OOM among them is latched for oomed().
        injected.push(GR_GL_INVALID_VALUE);
        injected.push(GR_GL_OUT_OF_MEMORY);
        injected.push(GR_GL_INVALID_OPERATION);
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, injected.fCount == 0);
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, context->oomed());
        REPORTER_ASSERT(reporter, !context->oomed(), "oomed() wasn't cleared");

        // Errors fail an active verify ops scope.
        context->beginVerifyOps();
        REPORTER_ASSERT(reporter, !context->hasVerifiedOpFailed());
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, !context->hasVerifiedOpFailed());
        injected.push(GR_GL_INVALID_FRAMEBUFFER_OPERATION);
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, context->hasVerifiedOpFailed());
        context->endVerifyOps();
        REPORTER_ASSERT(reporter, !context->oomed());

        // An abandoned context does not touch the backend.
        context->abandonContext();
        injected.push(GR_GL_OUT_OF_MEMORY);
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, injected.fCount == 1);
        REPORTER_ASSERT(reporter, !context->oomed());
        injected.fCount = 0;
    }

    // The original context's GL state tracking is stale after using a second GrDirectContext on
    // the same GL context.
    ctxInfo.directContext()->resetContext();
}
#endif  // defined(SK_GL)
