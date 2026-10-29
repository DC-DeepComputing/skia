/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
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
namespace {

// Wraps a GrGLInterface so tests can inject GL errors. Errors in fQueue are returned by
// glGetError() before any real driver errors. If fTexSubImageError is set, the next
// glTexSubImage2D call queues that error (simulating a failed upload).
struct GLErrorInjector {
    GrGLFunction<GrGLGetErrorFn> fGetError;
    GrGLFunction<GrGLTexSubImage2DFn> fTexSubImage2D;
    std::array<GrGLenum, 4> fQueue = {};
    int fCount = 0;
    GrGLenum fTexSubImageError = GR_GL_NO_ERROR;
    int fTexSubImageCalls = 0;

    void push(GrGLenum error) {
        SkASSERT_RELEASE(fCount < static_cast<int>(fQueue.size()));
        fQueue[fCount++] = error;
    }

    sk_sp<GrGLInterface> wrap(const GrGLInterface& base) {
        auto glInterface = sk_make_sp<GrGLInterface>(base);
        fGetError = glInterface->fFunctions.fGetError;
        fTexSubImage2D = glInterface->fFunctions.fTexSubImage2D;
        GLErrorInjector* self = this;
        glInterface->fFunctions.fGetError = [self]() -> GrGLenum {
            if (self->fCount > 0) {
                GrGLenum error = self->fQueue[0];
                for (int i = 1; i < self->fCount; ++i) {
                    self->fQueue[i - 1] = self->fQueue[i];
                }
                --self->fCount;
                return error;
            }
            return self->fGetError();
        };
        glInterface->fFunctions.fTexSubImage2D = [self](GrGLenum target,
                                                        GrGLint level,
                                                        GrGLint xoffset,
                                                        GrGLint yoffset,
                                                        GrGLsizei width,
                                                        GrGLsizei height,
                                                        GrGLenum format,
                                                        GrGLenum type,
                                                        const GrGLvoid* pixels) {
            ++self->fTexSubImageCalls;
            self->fTexSubImage2D(
                    target, level, xoffset, yoffset, width, height, format, type, pixels);
            if (self->fTexSubImageError != GR_GL_NO_ERROR) {
                self->push(self->fTexSubImageError);
                self->fTexSubImageError = GR_GL_NO_ERROR;
            }
        };
#if GR_GL_CHECK_ERROR
        // Suppress logging GL errors since we'll be synthetically generating them.
        glInterface->suppressErrorLogging();
#endif
        return glInterface;
    }
};

// Uploads to `surface` via SkCanvas::writePixels, flushes and submits, and returns whether the
// write succeeded. The upload is deferred to a render task that runs during the flush.
bool write_pixels(GrDirectContext* context, SkSurface* surface) {
    SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    bitmap.eraseColor(SK_ColorRED);
    bool written = surface->getCanvas()->writePixels(bitmap, 0, 0);
    return context->flushAndSubmit(surface, GrSyncCpu::kNo).fSuccess && written;
}

}  // namespace

// Verifies that upload error checks are only enabled by an explicit
// fSkipGLResourceErrorChecks = kNo, that they are independent of command error checks (per-submit
// drains), that a failed upload then makes the write fail, and that errors are latched by oomed()
// and checkAndResetBackendError().
DEF_GANESH_TEST_FOR_GL_CONTEXT(GrContext_GLErrorCheckGroups,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kNextRelease) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    glCtx->makeCurrent();

    auto run = [&](const char* label,
                   GrContextOptions::Enable skipAll,
                   GrContextOptions::Enable skipResource,
                   GrContextOptions::Enable skipCommand,
                   bool expectUploadChecks,
                   bool expectCommandChecks) {
        // This must outlive the GrDirectContext below, which captures it by pointer.
        GLErrorInjector injector;
        sk_sp<GrGLInterface> glInterface = injector.wrap(*glCtx->gl());
        GrContextOptions options;
        options.fSkipGLErrorChecks = skipAll;
        options.fSkipGLResourceErrorChecks = skipResource;
        options.fSkipGLCommandErrorChecks = skipCommand;
        sk_sp<GrDirectContext> context = GrDirectContexts::MakeGL(glInterface, options);
        if (!context) {
            ERRORF(reporter, "%s: Could not create GrDirectContext.", label);
            return;
        }
        SkImageInfo info = SkImageInfo::Make(8, 8, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(context.get(), skgpu::Budgeted::kNo, info);
        if (!surface) {
            ERRORF(reporter, "%s: Could not create surface.", label);
            return;
        }
        context->flushAndSubmit(GrSyncCpu::kYes);
        // Start from a clean slate.
        context->oomed();
        context->checkAndResetBackendError();

        // A successful upload succeeds and records no errors.
        REPORTER_ASSERT(reporter, write_pixels(context.get(), surface.get()), "%s", label);
        REPORTER_ASSERT(reporter, injector.fTexSubImageCalls > 0, "%s", label);
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError(), "%s", label);
        REPORTER_ASSERT(reporter, !context->oomed(), "%s", label);

        // A non-OOM error from the upload fails the write and is latched as a backend error.
        injector.fTexSubImageError = GR_GL_INVALID_OPERATION;
        bool wrote = write_pixels(context.get(), surface.get());
        REPORTER_ASSERT(reporter, injector.fTexSubImageError == GR_GL_NO_ERROR, "%s", label);
        if (expectUploadChecks) {
            REPORTER_ASSERT(reporter, !wrote, "%s", label);
            REPORTER_ASSERT(reporter, injector.fCount == 0, "%s", label);
            REPORTER_ASSERT(reporter, context->checkAndResetBackendError(), "%s", label);
            REPORTER_ASSERT(reporter, !context->checkAndResetBackendError(), "%s", label);
            REPORTER_ASSERT(reporter, !context->oomed(), "%s", label);
        } else if (expectCommandChecks) {
            // The upload isn't checked, but the error is drained at submit.
            REPORTER_ASSERT(reporter, wrote, "%s", label);
            REPORTER_ASSERT(reporter, injector.fCount == 0, "%s", label);
            REPORTER_ASSERT(reporter, context->checkAndResetBackendError(), "%s", label);
        } else {
            // Not drained by Skia, so not latched. (In GR_GL_CHECK_ERROR builds the error may
            // still have been consumed by GrGLInterface's per-call error check.)
            REPORTER_ASSERT(reporter, wrote, "%s", label);
            REPORTER_ASSERT(reporter, !context->checkAndResetBackendError(), "%s", label);
            injector.fCount = 0;
        }

        // An OOM from the upload fails the write and is latched by oomed().
        injector.fTexSubImageError = GR_GL_OUT_OF_MEMORY;
        wrote = write_pixels(context.get(), surface.get());
        if (expectUploadChecks) {
            REPORTER_ASSERT(reporter, !wrote, "%s", label);
            REPORTER_ASSERT(reporter, context->oomed(), "%s", label);
            REPORTER_ASSERT(reporter, !context->oomed(), "%s", label);
            REPORTER_ASSERT(reporter, !context->checkAndResetBackendError(), "%s", label);
        } else {
            REPORTER_ASSERT(reporter, wrote, "%s", label);
            REPORTER_ASSERT(reporter, context->oomed() == expectCommandChecks, "%s", label);
            injector.fCount = 0;
        }

        // A pending error is drained at submit only if command checks are enabled.
        injector.push(GR_GL_INVALID_VALUE);
        surface->getCanvas()->clear(SK_ColorBLUE);
        context->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        if (expectCommandChecks) {
            REPORTER_ASSERT(reporter, injector.fCount == 0, "%s", label);
            REPORTER_ASSERT(reporter, context->checkAndResetBackendError(), "%s", label);
        } else {
            REPORTER_ASSERT(reporter, !context->checkAndResetBackendError(), "%s", label);
            injector.fCount = 0;
        }
        REPORTER_ASSERT(reporter, !context->oomed(), "%s", label);

        surface.reset();
        context->flushAndSubmit(GrSyncCpu::kYes);
    };

    using Enable = GrContextOptions::Enable;
    // Uploads are checked only when fSkipGLResourceErrorChecks is explicitly kNo.
    run("resource on, command off", Enable::kDefault, Enable::kNo, Enable::kYes, true, false);
    run("resource default, command off", Enable::kDefault, Enable::kDefault, Enable::kYes, false,
        false);
    run("resource off, command on", Enable::kDefault, Enable::kYes, Enable::kNo, false, true);
    run("all on (deprecated option)", Enable::kNo, Enable::kDefault, Enable::kDefault, false, true);
    run("all off", Enable::kYes, Enable::kDefault, Enable::kDefault, false, false);
    run("all off, resource on", Enable::kYes, Enable::kNo, Enable::kDefault, true, false);

    // The original context's GL state tracking is stale after using a second GrDirectContext on
    // the same GL context.
    ctxInfo.directContext()->resetContext();
}

// Verifies that GrDirectContext::checkAndClearBackendErrors() drains client-generated GL errors and
// latches them so that oomed() and checkAndResetBackendError() report them.
DEF_GANESH_TEST_FOR_GL_CONTEXT(GrContext_checkAndClearBackendErrors,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kNextRelease) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    glCtx->makeCurrent();
    // This must outlive the GrDirectContext below, which captures it by pointer.
    GLErrorInjector injector;
    sk_sp<GrGLInterface> glInterface = injector.wrap(*glCtx->gl());

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
        context->checkAndResetBackendError();
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError());

        // A real error generated by a client GL call on Skia's context is drained and latched.
        glInterface->fFunctions.fActiveTexture(0);  // GL_INVALID_ENUM
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, context->checkAndResetBackendError());
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError());
        REPORTER_ASSERT(reporter, !context->oomed());

        // Multiple errors are all drained, and an OOM among them is latched for oomed().
        injector.push(GR_GL_INVALID_VALUE);
        injector.push(GR_GL_OUT_OF_MEMORY);
        injector.push(GR_GL_INVALID_OPERATION);
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, injector.fCount == 0);
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, context->oomed());
        REPORTER_ASSERT(reporter, !context->oomed(), "oomed() wasn't cleared");
        REPORTER_ASSERT(reporter, context->checkAndResetBackendError());
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError());

        // An OOM alone does not set the non-OOM backend error.
        injector.push(GR_GL_OUT_OF_MEMORY);
        REPORTER_ASSERT(reporter, context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, context->oomed());
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError());

        // An abandoned context does not touch the backend.
        context->abandonContext();
        injector.push(GR_GL_OUT_OF_MEMORY);
        REPORTER_ASSERT(reporter, !context->checkAndClearBackendErrors());
        REPORTER_ASSERT(reporter, injector.fCount == 1);
        REPORTER_ASSERT(reporter, !context->oomed());
        REPORTER_ASSERT(reporter, !context->checkAndResetBackendError());
        injector.fCount = 0;
    }

    // The original context's GL state tracking is stale after using a second GrDirectContext on
    // the same GL context.
    ctxInfo.directContext()->resetContext();
}
#endif  // defined(SK_GL)
