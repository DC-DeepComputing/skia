/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Reproduction and regression test for b/532941869.
//
// Imagination GL drivers can keep an internal reference to the previously bound (complete)
// framebuffer and touch it after it has been deleted, which turns into a use-after-free inside the
// driver. Chromium works around the same bug (gpu_driver_bug_list.json entry 471: "IMG drivers can
// sometimes reference previously bound complete framebuffers"). Skia's
// ensure_previous_framebuffer_not_deleted workaround does what Chromium's GLES2DecoderImpl does:
//   - GrGLGpu::bindFramebuffer() binds a never-deleted 1x1 complete FBO before every framebuffer it
//     binds, so a Skia FBO is never left behind as the previous binding of either target.
//   - GrGLGpu::deleteFramebuffer() always rebinds FBO 0 first, so the FBO being deleted is not the
//     current binding of the draw or the read target either.
//
// The invariant checked here: an FBO is never deleted while it is the current *or* the previous
// binding of GL_DRAW_FRAMEBUFFER or GL_READ_FRAMEBUFFER.
//
// 1) GLFBODeletion_*: Skia-level scenarios on a GrDirectContext whose GrGLInterface hooks
//    glBindFramebuffer and glDeleteFramebuffers. The hooks track the current and previous binding
//    of each target and check every deletion, whether Skia or the client makes it.
//      - GLFBODeletion_Workaround forces the workaround on, on every GPU, and fails on any
//        violation. It doesn't need Imagination hardware.
//      - GLFBODeletion_NoWorkaround disables driver workarounds and only logs. On an affected
//        device it is expected to reproduce the driver UAF.
//    Deleting a client-owned FBO while Skia still has it bound is up to the client to avoid.
//
// 2) GLFBODriverProbe_*: raw GLES call sequences that isolate which pattern the driver mishandles,
//    independent of Skia. They don't check anything; on an affected device the unsafe ones crash.
//    The driver isn't instrumented, so freed driver memory is most reliably caught with ASAN's
//    free-fill (ASAN_OPTIONS=detect_leaks=0:max_free_fill_size=65536) or with MTE. Run the probes
//    one at a time (--match '^GLFBODriverProbe_DeleteBoundRead$') so that a crash can be attributed
//    to a single pattern.

#include "tests/Test.h"

#ifdef SK_GL

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/gl/GrGLCaps.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tests/CtsEnforcement.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace {

// GL_READ_FRAMEBUFFER_BINDING (GL/GLES 3.0 and the framebuffer_blit extensions).
// GR_GL_FRAMEBUFFER_BINDING has the same value as GL_DRAW_FRAMEBUFFER_BINDING.
constexpr GrGLenum kReadFramebufferBinding = 0x8CAA;

constexpr int kSkiaIterations = 32;
constexpr int kProbeIterations = 256;
constexpr int kProbeSize = 64;

// GrGLGpu doesn't call glCheckFramebufferStatus under TSAN (see skbug.com/40043725); neither do we.
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
constexpr bool kCanCheckFramebufferStatus = false;
#else
constexpr bool kCanCheckFramebufferStatus = true;
#endif
#else
constexpr bool kCanCheckFramebufferStatus = true;
#endif

bool has_separate_read_draw_targets(const GrGLInterface* gl) {
    // GL/GLES 3.0, or one of the GLES 2.0 extensions that add GL_READ_FRAMEBUFFER and
    // GL_DRAW_FRAMEBUFFER (EXT/ANGLE/NV_framebuffer_blit, APPLE_framebuffer_multisample).
    return GrGLGetVersion(gl) >= GR_GL_VER(3, 0) ||
           static_cast<bool>(gl->fFunctions.fBlitFramebuffer) ||
           static_cast<bool>(gl->fFunctions.fResolveMultisampleFramebuffer);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Raw GL helpers, used by the "client" in the Skia scenarios and by the driver probes.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Makes sure TexImage2D/ReadPixels don't read from or write into a pixel buffer object.
void unbind_pixel_buffers(const GrGLInterface* gl) {
    const bool hasPBOs = gl->fStandard == kGL_GrGLStandard
                                 ? GrGLGetVersion(gl) >= GR_GL_VER(2, 1)
                                 : GrGLGetVersion(gl) >= GR_GL_VER(3, 0);
    if (hasPBOs) {
        GR_GL_CALL(gl, BindBuffer(GR_GL_PIXEL_PACK_BUFFER, 0));
        GR_GL_CALL(gl, BindBuffer(GR_GL_PIXEL_UNPACK_BUFFER, 0));
    }
}

// Creates an uninitialized size x size RGBA texture.
GrGLuint make_texture(const GrGLInterface* gl, int size) {
    GrGLuint tex = 0;
    GR_GL_CALL(gl, GenTextures(1, &tex));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, tex));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GR_GL_CALL(gl, TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA, size, size, 0, GR_GL_RGBA,
                              GR_GL_UNSIGNED_BYTE, nullptr));
    GR_GL_CALL(gl, BindTexture(GR_GL_TEXTURE_2D, 0));
    return tex;
}

// Creates an FBO with 'tex' as its color attachment and leaves it bound to GL_FRAMEBUFFER.
// Returns 0 if the FBO is incomplete.
GrGLuint make_fbo(const GrGLInterface* gl, GrGLuint tex) {
    GrGLuint fbo = 0;
    GR_GL_CALL(gl, GenFramebuffers(1, &fbo));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, fbo));
    GR_GL_CALL(gl, FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_TEXTURE_2D, tex, 0));
    if (kCanCheckFramebufferStatus) {
        GrGLenum status;
        GR_GL_CALL_RET(gl, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            GR_GL_CALL(gl, DeleteFramebuffers(1, &fbo));
            return 0;
        }
    }
    return fbo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Part 1: Skia-level scenarios with observed framebuffer bindings and deletions.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Tracks the current and previous binding of GL_DRAW_FRAMEBUFFER and GL_READ_FRAMEBUFFER through a
// hooked GrGLInterface, and records both at every glDeleteFramebuffers call.
class FBOBindingWatcher {
public:
    // A binding we know nothing about. Never matches a real FBO.
    static constexpr GrGLuint kUnknownFBO = ~0u;

    struct Binding {
        GrGLuint fCurrent = kUnknownFBO;
        GrGLuint fPrevious = kUnknownFBO;
    };

    struct Deletion {
        GrGLuint fFBO;
        Binding fDraw;
        Binding fRead;

        bool currentDraw() const { return fFBO == fDraw.fCurrent; }
        bool currentRead() const { return fFBO == fRead.fCurrent; }
        bool previousDraw() const { return fFBO == fDraw.fPrevious; }
        bool previousRead() const { return fFBO == fRead.fPrevious; }
        bool unsafe() const {
            return this->currentDraw() || this->currentRead() || this->previousDraw() ||
                   this->previousRead();
        }
    };

    // Returns a copy of 'base' that routes glBindFramebuffer and glDeleteFramebuffers through this
    // watcher. The watcher must outlive any GrDirectContext created with the returned interface,
    // and every framebuffer bind or delete must go through the returned interface.
    sk_sp<const GrGLInterface> wrap(sk_sp<const GrGLInterface> base) {
        fBase = std::move(base);
        fSeparateTargets = has_separate_read_draw_targets(fBase.get());
        sk_sp<GrGLInterface> hooked(new GrGLInterface(*fBase));
        FBOBindingWatcher* self = this;
        hooked->fFunctions.fBindFramebuffer = [self](GrGLenum target, GrGLuint fbo) {
            self->onBindFramebuffer(target, fbo);
        };
        hooked->fFunctions.fDeleteFramebuffers = [self](GrGLsizei n, const GrGLuint* fbos) {
            self->onDeleteFramebuffers(n, fbos);
        };
        GrGLuint draw, read;
        this->queryBindings(&draw, &read);
        fDraw = {draw, kUnknownFBO};
        fRead = {read, kUnknownFBO};
        return hooked;
    }

    const std::vector<Deletion>& deletions() const { return fDeletions; }

    // How often the tracked bindings didn't match GL, i.e. something bound a framebuffer without
    // going through the hooked interface. The results aren't reliable if this is non-zero.
    int desyncCount() const { return fDesyncCount; }

private:
    void queryBindings(GrGLuint* draw, GrGLuint* read) const {
        GrGLint d = 0;
        GrGLint r = 0;
        fBase->fFunctions.fGetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &d);
        if (fSeparateTargets) {
            fBase->fFunctions.fGetIntegerv(kReadFramebufferBinding, &r);
        } else {
            r = d;  // Without separate targets GL_FRAMEBUFFER is both.
        }
        *draw = static_cast<GrGLuint>(d);
        *read = static_cast<GrGLuint>(r);
    }

    static void Bind(Binding* binding, GrGLuint fbo) {
        // Conservatively assume that a redundant bind doesn't replace the driver's previous FBO.
        if (binding->fCurrent != fbo) {
            binding->fPrevious = binding->fCurrent;
            binding->fCurrent = fbo;
        }
    }

    // Once 'fbo' is deleted GL rebinds 0 if it was bound, and its name can be reused.
    static void Forget(Binding* binding, GrGLuint fbo) {
        if (binding->fCurrent == fbo) {
            binding->fCurrent = 0;
            binding->fPrevious = kUnknownFBO;
        } else if (binding->fPrevious == fbo) {
            binding->fPrevious = kUnknownFBO;
        }
    }

    void onBindFramebuffer(GrGLenum target, GrGLuint fbo) {
        fBase->fFunctions.fBindFramebuffer(target, fbo);
        if (target == GR_GL_FRAMEBUFFER || !fSeparateTargets) {
            Bind(&fDraw, fbo);
            Bind(&fRead, fbo);
        } else if (target == GR_GL_DRAW_FRAMEBUFFER) {
            Bind(&fDraw, fbo);
        } else if (target == GR_GL_READ_FRAMEBUFFER) {
            Bind(&fRead, fbo);
        }
    }

    void onDeleteFramebuffers(GrGLsizei n, const GrGLuint* fbos) {
        GrGLuint draw, read;
        this->queryBindings(&draw, &read);
        if (draw != fDraw.fCurrent) {
            ++fDesyncCount;
            fDraw = {draw, kUnknownFBO};
        }
        if (read != fRead.fCurrent) {
            ++fDesyncCount;
            fRead = {read, kUnknownFBO};
        }
        for (GrGLsizei i = 0; i < n; ++i) {
            if (fbos[i]) {  // glDeleteFramebuffers silently ignores 0.
                fDeletions.push_back({fbos[i], fDraw, fRead});
            }
        }
        fBase->fFunctions.fDeleteFramebuffers(n, fbos);
        for (GrGLsizei i = 0; i < n; ++i) {
            if (fbos[i]) {
                Forget(&fDraw, fbos[i]);
                Forget(&fRead, fbos[i]);
            }
        }
    }

    sk_sp<const GrGLInterface> fBase;
    bool fSeparateTargets = false;
    Binding fDraw;
    Binding fRead;
    std::vector<Deletion> fDeletions;
    int fDesyncCount = 0;
};

SkImageInfo surface_info() {
    return SkImageInfo::Make(128, 128, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
}

void draw_content(SkCanvas* canvas, int i) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setColor((i & 1) ? SK_ColorRED : SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(8 + (i % 16), 8, 64, 64), paint);
}

// Scenarios get the GrDirectContext and the hooked interface for the GL calls the "client" makes.
// The client calls resetContext() after making GL calls behind Skia's back. A scenario returns
// false if it isn't supported.
using ScenarioFn = bool (*)(GrDirectContext*, const GrGLInterface* clientGL);

enum class BeforeDelete { kNothing, kResetContext, kClientBindsZero };

bool draw_flush_and_delete(GrDirectContext* dContext,
                           const GrGLInterface* clientGL,
                           BeforeDelete beforeDelete) {
    for (int i = 0; i < kSkiaIterations; ++i) {
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, surface_info());
        if (!surface) {
            return false;
        }
        draw_content(surface->getCanvas(), i);
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        switch (beforeDelete) {
            case BeforeDelete::kNothing:
                break;
            case BeforeDelete::kResetContext:
                dContext->resetContext();
                break;
            case BeforeDelete::kClientBindsZero:
                GR_GL_CALL(clientGL, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
                dContext->resetContext(kRenderTarget_GrGLBackendState);
                break;
        }
        surface.reset();
        // Purges the render target: GrGLRenderTarget::onRelease() -> GrGLGpu::deleteFramebuffer().
        dContext->freeGpuResources();
    }
    return true;
}

// The originally reported pattern: Skia deletes the FBO it last drew to. The landed workaround
// bound FBO 0 first, which still left the deleted FBO as the previous binding.
bool scenario_delete_last_drawn(GrDirectContext* dContext, const GrGLInterface* clientGL) {
    return draw_flush_and_delete(dContext, clientGL, BeforeDelete::kNothing);
}

// The client calls resetContext() between the flush and the delete without touching framebuffer
// bindings (e.g. Chrome's PessimisticallyResetGrContext(), or a client that made unrelated GL
// calls). GrGLGpu::onResetContext() assumes FBO 0 is bound without binding it, so the landed
// workaround deleted the FBO while it was still bound.
bool scenario_delete_after_reset_context(GrDirectContext* dContext,
                                         const GrGLInterface* clientGL) {
    return draw_flush_and_delete(dContext, clientGL, BeforeDelete::kResetContext);
}

// The client binds FBO 0 itself (e.g. to draw to its window) and calls resetContext() before Skia
// deletes its FBO. The FBO is no longer bound, but it is the previous binding.
bool scenario_delete_after_client_unbind(GrDirectContext* dContext,
                                         const GrGLInterface* clientGL) {
    return draw_flush_and_delete(dContext, clientGL, BeforeDelete::kClientBindsZero);
}

// Scaling copies into a non-renderable texture. A renderable dst would be copied with a draw, so
// the only way GrGLGpu::onCopySurface() can do these is glBlitFramebuffer, which leaves the source
// FBO bound to GL_READ_FRAMEBUFFER (and GrGLGpu's temporary dst FBO bound to GL_DRAW_FRAMEBUFFER).
// A renderable source uses its own FBO; a non-renderable one uses GrGLGpu's temporary src FBO.
bool blit_into_non_renderable(GrDirectContext* dContext,
                              GrRenderable srcRenderable,
                              int iterations) {
    // createTexture() and copySurface() have to be called through GrGpu: GrGLGpu declares a
    // private createTexture() that hides the GrGpu overloads.
    GrGpu* gpu = dContext->priv().getGpu();
    auto glGpu = static_cast<GrGLGpu*>(gpu);
    const GrBackendFormat format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
    const SkIRect srcRect = SkIRect::MakeWH(64, 64);
    const SkIRect dstRect = SkIRect::MakeWH(32, 32);
    const GrTextureType texType = GrTextureType::k2D;
    const int srcSampleCnt = srcRenderable == GrRenderable::kYes ? 1 : 0;
    if (!glGpu->glCaps().canCopyAsBlit(GrGLFormat::kRGBA8, /*dstSampleCnt=*/0, &texType,
                                       GrGLFormat::kRGBA8, srcSampleCnt, &texType,
                                       SkRect::Make(srcRect), /*srcBoundsExact=*/true,
                                       srcRect, dstRect)) {
        return false;
    }

    sk_sp<GrTexture> dst = gpu->createTexture(dstRect.size(), format, texType, GrRenderable::kNo,
                                              1, skgpu::Mipmapped::kNo, skgpu::Budgeted::kNo,
                                              GrProtected::kNo, "GLFBODeletion_BlitDst");
    if (!dst) {
        return false;
    }
    for (int i = 0; i < iterations; ++i) {
        sk_sp<GrTexture> src = gpu->createTexture(srcRect.size(), format, texType, srcRenderable,
                                                  1, skgpu::Mipmapped::kNo, skgpu::Budgeted::kNo,
                                                  GrProtected::kNo, "GLFBODeletion_BlitSrc");
        if (!src) {
            return false;
        }
        if (!gpu->copySurface(dst.get(), dstRect, src.get(), srcRect,
                              GrSamplerState::Filter::kLinear)) {
            return false;
        }
        src.reset();
        dContext->freeGpuResources();
    }
    return true;
}

// A renderable source's FBO is deleted while it is still bound to GL_READ_FRAMEBUFFER, which the
// landed workaround didn't track. MSAA (non-MSRTT) snapshot copies end up in the same state.
bool scenario_delete_blit_source(GrDirectContext* dContext, const GrGLInterface*) {
    return blit_into_non_renderable(dContext, GrRenderable::kYes, kSkiaIterations);
}

// GrGLGpu's temporary src FBO stays bound to GL_READ_FRAMEBUFFER until it is deleted along with the
// context (and the temporary dst FBO stays bound to GL_DRAW_FRAMEBUFFER).
bool scenario_delete_temp_fbos_on_teardown(GrDirectContext* dContext, const GrGLInterface*) {
    return blit_into_non_renderable(dContext, GrRenderable::kNo, 1);
}

// Resolving an MSAA surface (without EXT_multisampled_render_to_texture) blits from its multisample
// FBO, bound to GL_READ_FRAMEBUFFER, into its resolve FBO, bound to GL_DRAW_FRAMEBUFFER. Releasing
// the surface then deletes both while they are still bound.
bool scenario_delete_msaa_resolve_fbos(GrDirectContext* dContext, const GrGLInterface*) {
    const int sampleCount =
            std::min(4, dContext->maxSurfaceSampleCountForColorType(kRGBA_8888_SkColorType));
    if (sampleCount < 2) {
        return false;
    }
    for (int i = 0; i < kSkiaIterations; ++i) {
        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
                dContext, skgpu::Budgeted::kNo, surface_info(), sampleCount, nullptr);
        if (!surface) {
            return false;
        }
        draw_content(surface->getCanvas(), i);
        // Flushing the surface resolves it.
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        surface.reset();
        dContext->freeGpuResources();
    }
    return true;
}

// The client wraps its own FBO and deletes it once Skia has moved on to another surface, so the FBO
// is no longer bound. That is only safe if Skia didn't leave it behind as the previous binding.
bool scenario_client_deletes_wrapped_fbo(GrDirectContext* dContext,
                                         const GrGLInterface* clientGL) {
    const SkImageInfo info = surface_info();
    sk_sp<SkSurface> other = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
    if (!other) {
        return false;
    }
    for (int i = 0; i < kSkiaIterations; ++i) {
        unbind_pixel_buffers(clientGL);
        GrGLuint tex = make_texture(clientGL, info.width());
        GrGLuint fbo = make_fbo(clientGL, tex);
        dContext->resetContext();
        if (!fbo) {
            GR_GL_CALL(clientGL, DeleteTextures(1, &tex));
            dContext->resetContext();
            return false;
        }

        GrGLFramebufferInfo fboInfo;
        fboInfo.fFBOID = fbo;
        fboInfo.fFormat = GR_GL_RGBA8;
        GrBackendRenderTarget backendRT = GrBackendRenderTargets::MakeGL(
                info.width(), info.height(), /*sampleCnt=*/1, /*stencilBits=*/0, fboInfo);
        sk_sp<SkSurface> wrapped = SkSurfaces::WrapBackendRenderTarget(
                dContext, backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                /*colorSpace=*/nullptr, /*surfaceProps=*/nullptr);
        const bool ok = static_cast<bool>(wrapped);
        if (ok) {
            draw_content(wrapped->getCanvas(), i);
            dContext->flushAndSubmit(wrapped.get(), GrSyncCpu::kNo);
            wrapped.reset();  // Borrowed, so Skia doesn't delete the FBO.
            // Skia moves on to another surface.
            draw_content(other->getCanvas(), i);
            dContext->flushAndSubmit(other.get(), GrSyncCpu::kNo);
        }

        GR_GL_CALL(clientGL, DeleteFramebuffers(1, &fbo));
        GR_GL_CALL(clientGL, DeleteTextures(1, &tex));
        dContext->resetContext();
        if (!ok) {
            return false;
        }
    }
    return true;
}

// GrGpu's testing-only backend render target. Creating the first one probes for a compatible
// stencil format (GrGLGpu::getCompatibleStencilIndex()), which creates and deletes FBOs as well.
// The client wraps it and draws to it, then Skia deletes the FBO while it is still bound.
bool scenario_delete_testing_only_backend_rt(GrDirectContext* dContext, const GrGLInterface*) {
    GrGpu* gpu = dContext->priv().getGpu();
    const SkImageInfo info = surface_info();
    for (int i = 0; i < kSkiaIterations; ++i) {
        GrBackendRenderTarget backendRT = gpu->createTestingOnlyBackendRenderTarget(
                info.dimensions(), GrColorType::kRGBA_8888);
        if (!backendRT.isValid()) {
            return false;
        }
        sk_sp<SkSurface> surface = SkSurfaces::WrapBackendRenderTarget(
                dContext, backendRT, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                /*colorSpace=*/nullptr, /*surfaceProps=*/nullptr);
        const bool ok = static_cast<bool>(surface);
        if (ok) {
            draw_content(surface->getCanvas(), i);
            dContext->flushAndSubmit(surface.get(), GrSyncCpu::kYes);
            surface.reset();
        }
        gpu->deleteTestingOnlyBackendRenderTarget(backendRT);
        if (!ok) {
            return false;
        }
    }
    return true;
}

struct SkiaScenario {
    const char* fName;
    ScenarioFn fRun;
};

constexpr SkiaScenario kSkiaScenarios[] = {
        {"DeleteLastDrawn", scenario_delete_last_drawn},
        {"DeleteAfterResetContext", scenario_delete_after_reset_context},
        {"DeleteAfterClientUnbind", scenario_delete_after_client_unbind},
        {"DeleteBlitSource", scenario_delete_blit_source},
        {"DeleteTempFBOsOnTeardown", scenario_delete_temp_fbos_on_teardown},
        {"DeleteMSAAResolveFBOs", scenario_delete_msaa_resolve_fbos},
        {"ClientDeletesWrappedFBO", scenario_client_deletes_wrapped_fbo},
        {"DeleteTestingOnlyBackendRT", scenario_delete_testing_only_backend_rt},
};

enum class WorkaroundMode { kForceOn, kAllDisabled };

// With the workaround, every deletion should see GrGLGpu's complete FBO as the previous binding of
// both targets. Checks that it is one and the same FBO, and that it really is complete: the driver
// bug is about previously bound *complete* framebuffers.
void check_previous_fbo_is_complete(skiatest::Reporter* reporter,
                                    const char* name,
                                    GrDirectContext* dContext,
                                    const GrGLInterface* gl,
                                    const FBOBindingWatcher& watcher) {
    constexpr GrGLuint kUnknown = FBOBindingWatcher::kUnknownFBO;
    GrGLuint previous = kUnknown;
    for (const FBOBindingWatcher::Deletion& d : watcher.deletions()) {
        const GrGLuint previousBindings[] = {d.fDraw.fPrevious, d.fRead.fPrevious};
        for (GrGLuint p : previousBindings) {
            if (p == kUnknown) {
                continue;
            }
            if (previous != kUnknown && p != previous) {
                ERRORF(reporter, "%s: expected FBO %u as the previous binding, found FBO %u", name,
                       previous, p);
                return;
            }
            previous = p;
        }
    }
    if (previous == kUnknown) {
        return;  // Nothing has been deleted yet; it all happens when the context is destroyed.
    }
    if (previous == 0) {
        ERRORF(reporter, "%s: FBO 0 was the previous binding when an FBO was deleted", name);
        return;
    }
    if (!kCanCheckFramebufferStatus) {
        return;
    }
    // Binding 'previous' and then 0 leaves the same state GrGLGpu leaves behind after a deletion.
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, previous));
    GrGLenum status;
    GR_GL_CALL_RET(gl, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    GR_GL_CALL(gl, BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
    dContext->resetContext(kRenderTarget_GrGLBackendState);
    if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
        ERRORF(reporter, "%s: the FBO bound before every other FBO (%u) is incomplete (0x%x)", name,
               previous, status);
    }
}

void run_skia_scenarios(skiatest::Reporter* reporter,
                        const sk_gpu_test::ContextInfo& ctxInfo,
                        WorkaroundMode mode) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    // We share the GL context with the factory's GrDirectContext. Get it out of the way first.
    ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);

    GrContextOptions options = ctxInfo.options();
    if (mode == WorkaroundMode::kForceOn) {
        // Enable the Imagination workaround on every GPU so the test is meaningful everywhere.
        options.fDriverBugWorkarounds.ensure_previous_framebuffer_not_deleted = true;
    } else {
        // There is no way to force a single workaround off, so disable all of them.
        options.fDisableDriverCorrectnessWorkarounds = true;
    }

    for (const SkiaScenario& scenario : kSkiaScenarios) {
        // Observes everything from context creation to destruction (which deletes GrGLGpu's
        // temporary FBOs), so it must outlive the context.
        FBOBindingWatcher watcher;
        bool ran = false;
        {
            sk_sp<const GrGLInterface> gl = watcher.wrap(sk_ref_sp(glCtx->gl()));
            sk_sp<GrDirectContext> dContext = GrDirectContexts::MakeGL(gl, options);
            if (!dContext) {
                ERRORF(reporter, "Could not create a GrDirectContext with the hooked interface");
                return;
            }
            auto gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());
            const bool enabled =
                    gpu->glCaps().workarounds().ensure_previous_framebuffer_not_deleted;
            if (enabled != (mode == WorkaroundMode::kForceOn)) {
                ERRORF(reporter, "ensure_previous_framebuffer_not_deleted is unexpectedly %s",
                       enabled ? "on" : "off");
                return;
            }
            ran = scenario.fRun(dContext.get(), gl.get());
            if (ran && mode == WorkaroundMode::kForceOn) {
                check_previous_fbo_is_complete(reporter, scenario.fName, dContext.get(), gl.get(),
                                               watcher);
            }
        }

        if (!ran) {
            INFOF(reporter, "%s: skipped (unsupported)\n", scenario.fName);
            continue;
        }
        if (watcher.desyncCount()) {
            ERRORF(reporter,
                   "%s: %d framebuffer binding changes bypassed the hooked interface, so the "
                   "results are unreliable",
                   scenario.fName, watcher.desyncCount());
        }

        int unsafe = 0;
        int currentDraw = 0;
        int currentRead = 0;
        int previousDraw = 0;
        int previousRead = 0;
        const FBOBindingWatcher::Deletion* firstUnsafe = nullptr;
        for (const FBOBindingWatcher::Deletion& d : watcher.deletions()) {
            currentDraw += d.currentDraw() ? 1 : 0;
            currentRead += d.currentRead() ? 1 : 0;
            previousDraw += d.previousDraw() ? 1 : 0;
            previousRead += d.previousRead() ? 1 : 0;
            if (d.unsafe()) {
                ++unsafe;
                if (!firstUnsafe) {
                    firstUnsafe = &d;
                }
            }
        }
        const int total = static_cast<int>(watcher.deletions().size());
        INFOF(reporter,
              "%s: %d FBO deletions, %d unsafe (current binding: %d draw, %d read; previous "
              "binding: %d draw, %d read)\n",
              scenario.fName, total, unsafe, currentDraw, currentRead, previousDraw, previousRead);

        if (mode == WorkaroundMode::kForceOn && firstUnsafe) {
            ERRORF(reporter,
                   "%s: ensure_previous_framebuffer_not_deleted is enabled, but %d of %d FBO "
                   "deletions deleted the current or previous binding. First: FBO %u, draw "
                   "binding %u (previous %u), read binding %u (previous %u)",
                   scenario.fName, unsafe, total, firstUnsafe->fFBO, firstUnsafe->fDraw.fCurrent,
                   firstUnsafe->fDraw.fPrevious, firstUnsafe->fRead.fCurrent,
                   firstUnsafe->fRead.fPrevious);
        }
    }

    // We made GL calls behind the factory context's back.
    glCtx->makeCurrent();
    ctxInfo.directContext()->resetContext();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Part 2: Raw GL driver probes.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Minimal raw GLES helper. Everything here is plain GL so the sequences can be handed to the
// driver vendor as-is.
class RawGL {
public:
    explicit RawGL(const GrGLInterface* gl) : fGL(gl) {}

    RawGL(const RawGL&) = delete;
    RawGL& operator=(const RawGL&) = delete;

    ~RawGL() {
        // Make sure nothing deleted here is the current or previous binding.
        this->unbindFBOSafely();
        if (fScratchFBO) {
            this->deleteFBO(fScratchFBO);
        }
        if (fIncompleteFBO) {
            this->deleteFBO(fIncompleteFBO);
        }
        if (fScratchTex) {
            this->deleteTexture(fScratchTex);
        }
        if (fVBO) {
            GR_GL_CALL(fGL, DeleteBuffers(1, &fVBO));
        }
        if (fProgram) {
            GR_GL_CALL(fGL, UseProgram(0));
            GR_GL_CALL(fGL, DeleteProgram(fProgram));
        }
    }

    bool init() {
        unbind_pixel_buffers(fGL);
        static constexpr char kVS[] =
                "attribute vec2 a_pos;\n"
                "void main() { gl_Position = vec4(a_pos, 0.0, 1.0); }\n";
        static constexpr char kFS[] =
                "precision mediump float;\n"
                "uniform vec4 u_color;\n"
                "void main() { gl_FragColor = u_color; }\n";
        GrGLuint vs = this->compile(GR_GL_VERTEX_SHADER, kVS);
        GrGLuint fs = this->compile(GR_GL_FRAGMENT_SHADER, kFS);
        if (!vs || !fs) {
            return false;
        }
        GR_GL_CALL_RET(fGL, fProgram, CreateProgram());
        GR_GL_CALL(fGL, AttachShader(fProgram, vs));
        GR_GL_CALL(fGL, AttachShader(fProgram, fs));
        GR_GL_CALL(fGL, BindAttribLocation(fProgram, 0, "a_pos"));
        GR_GL_CALL(fGL, LinkProgram(fProgram));
        GR_GL_CALL(fGL, DeleteShader(vs));
        GR_GL_CALL(fGL, DeleteShader(fs));
        GrGLint linked = GR_GL_FALSE;
        GR_GL_CALL(fGL, GetProgramiv(fProgram, GR_GL_LINK_STATUS, &linked));
        if (!linked) {
            return false;
        }
        GR_GL_CALL_RET(fGL, fColorLoc, GetUniformLocation(fProgram, "u_color"));

        static constexpr float kQuad[] = {-1, -1, 1, -1, -1, 1, 1, 1};
        GR_GL_CALL(fGL, GenBuffers(1, &fVBO));
        GR_GL_CALL(fGL, BindBuffer(GR_GL_ARRAY_BUFFER, fVBO));
        GR_GL_CALL(fGL, BufferData(GR_GL_ARRAY_BUFFER, sizeof(kQuad), kQuad, GR_GL_STATIC_DRAW));

        // The guard FBO and its texture are never deleted; see fGuardFBO.
        fGuardFBO = this->makeFBO(this->makeTexture());
        fScratchTex = this->makeTexture();
        fScratchFBO = this->makeFBO(fScratchTex);
        // An FBO is created when it is first bound. Without attachments it is never complete.
        GR_GL_CALL(fGL, GenFramebuffers(1, &fIncompleteFBO));
        this->bindFBO(GR_GL_FRAMEBUFFER, fIncompleteFBO);
        this->unbindFBOSafely();
        return fGuardFBO && fScratchFBO && fIncompleteFBO;
    }

    GrGLuint makeTexture() { return make_texture(fGL, kProbeSize); }

    // Leaves the new FBO bound to GL_FRAMEBUFFER.
    GrGLuint makeFBO(GrGLuint tex) { return make_fbo(fGL, tex); }

    void bindFBO(GrGLenum target, GrGLuint fbo) { GR_GL_CALL(fGL, BindFramebuffer(target, fbo)); }

    // What GrGLGpu does with ensure_previous_framebuffer_not_deleted: bind the complete guard FBO
    // first, so that it becomes the previous binding of 'target'.
    void bindAfterGuard(GrGLenum target, GrGLuint fbo) {
        this->bindFBO(target, fGuardFBO);
        this->bindFBO(target, fbo);
    }

    // The same, except that the FBO bound first is incomplete.
    void bindAfterIncompleteGuard(GrGLenum target, GrGLuint fbo) {
        this->bindFBO(target, fIncompleteFBO);
        this->bindFBO(target, fbo);
    }

    // Leaves FBO 0 bound to both targets, with the guard FBO as their previous binding. This is
    // the state GrGLGpu::deleteFramebuffer() deletes FBOs in.
    void unbindFBOSafely() { this->bindAfterGuard(GR_GL_FRAMEBUFFER, 0); }

    void deleteFBO(GrGLuint fbo) { GR_GL_CALL(fGL, DeleteFramebuffers(1, &fbo)); }

    void deleteTexture(GrGLuint tex) { GR_GL_CALL(fGL, DeleteTextures(1, &tex)); }

    // Records a clear plus a draw into the bound framebuffer, so that the driver has pending
    // (tiled) work referencing it.
    void drawBound(int i) {
        GR_GL_CALL(fGL, Viewport(0, 0, kProbeSize, kProbeSize));
        GR_GL_CALL(fGL, Disable(GR_GL_SCISSOR_TEST));
        GR_GL_CALL(fGL, Disable(GR_GL_BLEND));
        GR_GL_CALL(fGL, ColorMask(1, 1, 1, 1));
        GR_GL_CALL(fGL, ClearColor(0, 0, (i & 1) ? 1.f : 0.f, 1));
        GR_GL_CALL(fGL, Clear(GR_GL_COLOR_BUFFER_BIT));
        if (fGL->fFunctions.fBindVertexArray) {
            GR_GL_CALL(fGL, BindVertexArray(0));
        }
        GR_GL_CALL(fGL, UseProgram(fProgram));
        GR_GL_CALL(fGL, Uniform4f(fColorLoc, (i & 1) ? 1.f : 0.f, 1.f, 0.f, 1.f));
        GR_GL_CALL(fGL, BindBuffer(GR_GL_ARRAY_BUFFER, fVBO));
        GR_GL_CALL(fGL, VertexAttribPointer(0, 2, GR_GL_FLOAT, GR_GL_FALSE, 0, nullptr));
        GR_GL_CALL(fGL, EnableVertexAttribArray(0));
        GR_GL_CALL(fGL, DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    }

    // Binds 'fbo' to GL_FRAMEBUFFER and draws into it. Leaves 'fbo' bound.
    void drawInto(GrGLuint fbo, int i) {
        this->bindFBO(GR_GL_FRAMEBUFFER, fbo);
        this->drawBound(i);
    }

    // Blits 'src' (bound to GL_READ_FRAMEBUFFER) into 'dst' (bound to GL_DRAW_FRAMEBUFFER) and
    // leaves both bindings in place.
    void blit(GrGLuint src, GrGLuint dst) {
        this->bindFBO(GR_GL_READ_FRAMEBUFFER, src);
        this->bindFBO(GR_GL_DRAW_FRAMEBUFFER, dst);
        GR_GL_CALL(fGL, BlitFramebuffer(0, 0, kProbeSize, kProbeSize, 0, 0, kProbeSize / 2,
                                        kProbeSize / 2, GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
    }

    // Allocates and frees a few driver objects so freed framebuffer memory is likely to be reused,
    // then does work that makes the driver look at its framebuffer state (bind changes, a draw, a
    // flush, and every 16 iterations a readback and finish). The churn FBOs are deleted the safe
    // way so that they don't add unsafe deletions of their own.
    void pokeDriver(int i) {
        GrGLuint texs[4];
        GrGLuint fbos[4];
        for (int j = 0; j < 4; ++j) {
            texs[j] = this->makeTexture();
            fbos[j] = this->makeFBO(texs[j]);
        }
        this->unbindFBOSafely();
        for (int j = 0; j < 4; ++j) {
            if (fbos[j]) {
                this->deleteFBO(fbos[j]);
            }
            this->deleteTexture(texs[j]);
        }

        this->drawInto(fScratchFBO, i);
        GR_GL_CALL(fGL, Flush());
        if ((i % 16) == 15) {
            uint32_t pixel;
            GR_GL_CALL(fGL, ReadPixels(0, 0, 1, 1, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, &pixel));
            GR_GL_CALL(fGL, Finish());
        }
    }

private:
    GrGLuint compile(GrGLenum type, const char* src) {
        GrGLuint shader;
        GR_GL_CALL_RET(fGL, shader, CreateShader(type));
        GR_GL_CALL(fGL, ShaderSource(shader, 1, &src, nullptr));
        GR_GL_CALL(fGL, CompileShader(shader));
        GrGLint compiled = GR_GL_FALSE;
        GR_GL_CALL(fGL, GetShaderiv(shader, GR_GL_COMPILE_STATUS, &compiled));
        if (!compiled) {
            GR_GL_CALL(fGL, DeleteShader(shader));
            return 0;
        }
        return shader;
    }

    const GrGLInterface* fGL;
    GrGLuint fProgram = 0;
    GrGLint fColorLoc = -1;
    GrGLuint fVBO = 0;
    // A complete FBO used the way GrGLGpu uses its complete FBO. Like GrGLGpu's, it is never
    // deleted, since whichever FBO is deleted last would be left behind as the previous binding.
    // It goes away with the GL context.
    GrGLuint fGuardFBO = 0;
    GrGLuint fIncompleteFBO = 0;
    GrGLuint fScratchTex = 0;
    GrGLuint fScratchFBO = 0;
};

struct ProbeEnv {
    RawGL* fGL;
    sk_gpu_test::GLTestContext* fCtx;
    // Only set for probes that need a second context.
    RawGL* fOtherGL;
    sk_gpu_test::GLTestContext* fOtherCtx;
};

using ProbeFn = void (*)(const ProbeEnv&, int i);

// The original bug with no workaround: delete the FBO while it is bound for drawing.
void probe_delete_bound_draw(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.drawInto(fbo, i);
    gl.deleteFBO(fbo);  // Still bound to GL_FRAMEBUFFER.
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// What the landed workaround did: bind FBO 0, then delete. That leaves the deleted FBO as the
// previous binding.
void probe_bind_zero_then_delete(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.drawInto(fbo, i);
    gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(fbo);
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// Delete an FBO that is bound to GL_READ_FRAMEBUFFER only, like a blit or MSAA resolve source.
void probe_delete_bound_read(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint srcTex = gl.makeTexture();
    GrGLuint src = gl.makeFBO(srcTex);
    GrGLuint dstTex = gl.makeTexture();
    GrGLuint dst = gl.makeFBO(dstTex);
    gl.drawInto(src, i);
    gl.drawInto(dst, i + 1);
    gl.unbindFBOSafely();  // So 'src' isn't the previous draw binding as well.
    gl.blit(src, dst);
    gl.deleteFBO(src);  // Current GL_READ_FRAMEBUFFER binding.
    gl.pokeDriver(i);
    gl.unbindFBOSafely();
    gl.deleteFBO(dst);
    gl.deleteTexture(srcTex);
    gl.deleteTexture(dstTex);
}

// Delete the FBO that was bound to GL_FRAMEBUFFER before the current one.
void probe_delete_previously_bound(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint texA = gl.makeTexture();
    GrGLuint a = gl.makeFBO(texA);
    GrGLuint texB = gl.makeTexture();
    GrGLuint b = gl.makeFBO(texB);
    gl.drawInto(a, i);
    gl.drawInto(b, i + 1);
    gl.deleteFBO(a);  // Not bound, but it is the previous binding of both targets.
    gl.pokeDriver(i);
    gl.unbindFBOSafely();
    gl.deleteFBO(b);
    gl.deleteTexture(texA);
    gl.deleteTexture(texB);
}

// Delete the previous GL_READ_FRAMEBUFFER binding without touching the draw binding.
void probe_delete_previous_read(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint texA = gl.makeTexture();
    GrGLuint a = gl.makeFBO(texA);
    GrGLuint texB = gl.makeTexture();
    GrGLuint b = gl.makeFBO(texB);
    GrGLuint dstTex = gl.makeTexture();
    GrGLuint dst = gl.makeFBO(dstTex);
    gl.drawInto(a, i);
    gl.drawInto(b, i + 1);
    gl.unbindFBOSafely();
    gl.blit(a, dst);
    gl.blit(b, dst);
    gl.deleteFBO(a);  // Previous GL_READ_FRAMEBUFFER binding.
    gl.pokeDriver(i);
    gl.unbindFBOSafely();
    gl.deleteFBO(b);
    gl.deleteFBO(dst);
    gl.deleteTexture(texA);
    gl.deleteTexture(texB);
    gl.deleteTexture(dstTex);
}

// What GrGLGpu (and Chromium) do now: bind a complete guard FBO before every bind, and bind 0 the
// same way before deleting. Expected to be safe.
void probe_sandwich_with_complete_fbo(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.bindAfterGuard(GR_GL_FRAMEBUFFER, fbo);
    gl.drawBound(i);
    gl.unbindFBOSafely();
    gl.deleteFBO(fbo);
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// The same with an incomplete guard FBO. The driver bug is about previously bound *complete*
// framebuffers, so if this crashes while SandwichWithCompleteFBO doesn't, the guard FBO must be
// complete (which is why GrGLGpu gives it a 1x1 color attachment).
void probe_sandwich_with_incomplete_fbo(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.bindAfterIncompleteGuard(GR_GL_FRAMEBUFFER, fbo);
    gl.drawBound(i);
    gl.bindAfterIncompleteGuard(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(fbo);
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// The safe pattern, with eglMakeCurrent() switches while the FBO is bound and right after it is
// deleted. Chromium (unbind_fbo_on_context_switch) and ANGLE (unbindFBOBeforeSwitchingContext)
// have context switch workarounds for Imagination too. If this crashes while
// SandwichWithCompleteFBO doesn't, context switches matter, which Skia doesn't control.
void probe_sandwich_across_make_current(const ProbeEnv& env, int i) {
    RawGL& gl = *env.fGL;
    auto switchAndWork = [&] {
        env.fOtherCtx->makeCurrent();
        env.fOtherGL->pokeDriver(i);
        env.fCtx->makeCurrent();
    };
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.bindAfterGuard(GR_GL_FRAMEBUFFER, fbo);
    gl.drawBound(i);
    switchAndWork();
    gl.unbindFBOSafely();
    gl.deleteFBO(fbo);
    switchAndWork();
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

enum ProbeRequirements : int {
    kProbeNoRequirements = 0,
    kProbeNeedsBlit = 1 << 0,
    kProbeNeedsSecondContext = 1 << 1,
};

void run_driver_probe(skiatest::Reporter* reporter,
                      const sk_gpu_test::ContextInfo& ctxInfo,
                      ProbeFn probe,
                      int requirements) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    const GrGLInterface* gli = glCtx->gl();
    if (gli->fStandard != kGLES_GrGLStandard) {
        INFOF(reporter, "Driver probes only run on GLES\n");
        return;
    }
    if ((requirements & kProbeNeedsBlit) && !gli->fFunctions.fBlitFramebuffer) {
        INFOF(reporter, "Probe needs glBlitFramebuffer\n");
        return;
    }
    ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);

    std::unique_ptr<sk_gpu_test::GLTestContext> otherCtx;
    std::unique_ptr<RawGL> otherGL;
    if (requirements & kProbeNeedsSecondContext) {
        otherCtx = glCtx->makeNew();  // Makes the new context current.
        if (otherCtx) {
            otherGL = std::make_unique<RawGL>(otherCtx->gl());
            if (!otherGL->init()) {
                otherGL.reset();
            }
        }
        if (!otherGL) {
            otherCtx.reset();
            glCtx->makeCurrent();
            INFOF(reporter, "Probe needs a second GL context\n");
            return;
        }
        glCtx->makeCurrent();
    }

    {
        RawGL gl(gli);
        if (!gl.init()) {
            ERRORF(reporter, "Failed to set up raw GL state");
        } else {
            const ProbeEnv env = {&gl, glCtx, otherGL.get(), otherCtx.get()};
            for (int i = 0; i < kProbeIterations; ++i) {
                probe(env, i);
            }
            gl.unbindFBOSafely();
            GR_GL_CALL(gli, Finish());
        }
    }

    if (otherCtx) {
        otherCtx->makeCurrent();
        otherGL.reset();
        otherCtx.reset();
        glCtx->makeCurrent();
    }
    ctxInfo.directContext()->resetContext();
}

}  // anonymous namespace

DEF_GANESH_TEST_FOR_GL_CONTEXT(GLFBODeletion_Workaround, reporter, ctxInfo, CtsEnforcement::kNever) {
    run_skia_scenarios(reporter, ctxInfo, WorkaroundMode::kForceOn);
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(GLFBODeletion_NoWorkaround,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kNever) {
    run_skia_scenarios(reporter, ctxInfo, WorkaroundMode::kAllDisabled);
}

// No probe name is a substring of another, so --match can select a single probe.
#define DEF_FBO_DRIVER_PROBE(name, probe, requirements)          \
    DEF_GANESH_TEST_FOR_GL_CONTEXT(GLFBODriverProbe_##name,       \
                                   reporter,                      \
                                   ctxInfo,                       \
                                   CtsEnforcement::kNever) {      \
        run_driver_probe(reporter, ctxInfo, probe, requirements); \
    }

// Unsafe patterns; expected to crash on affected drivers.
DEF_FBO_DRIVER_PROBE(DeleteBoundDraw, probe_delete_bound_draw, kProbeNoRequirements)
DEF_FBO_DRIVER_PROBE(BindZeroThenDelete, probe_bind_zero_then_delete, kProbeNoRequirements)
DEF_FBO_DRIVER_PROBE(DeleteBoundRead, probe_delete_bound_read, kProbeNeedsBlit)
DEF_FBO_DRIVER_PROBE(DeletePreviouslyBound, probe_delete_previously_bound, kProbeNoRequirements)
DEF_FBO_DRIVER_PROBE(DeletePreviousRead, probe_delete_previous_read, kProbeNeedsBlit)
DEF_FBO_DRIVER_PROBE(SandwichWithIncompleteFBO,
                     probe_sandwich_with_incomplete_fbo,
                     kProbeNoRequirements)
// The pattern GrGLGpu uses now; expected to be safe.
DEF_FBO_DRIVER_PROBE(SandwichWithCompleteFBO,
                     probe_sandwich_with_complete_fbo,
                     kProbeNoRequirements)
DEF_FBO_DRIVER_PROBE(SandwichAcrossMakeCurrent,
                     probe_sandwich_across_make_current,
                     kProbeNeedsSecondContext)

#undef DEF_FBO_DRIVER_PROBE

#endif  // SK_GL
