/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Reproduction for b/532941869.
//
// Imagination GL drivers keep an internal reference to a framebuffer that is deleted while it is
// still bound, which later turns into a use-after-free inside the driver. Skia's workaround
// (ensure_previous_framebuffer_not_deleted) binds FBO 0 before deleting an FBO, but only when the
// FBO matches GrGLGpu::fBoundDrawFramebuffer, i.e. what Skia *believes* is bound for drawing.
//
// This file contains two groups of tests:
//
// 1) GLFBODeletion_*: Skia-level scenarios run on a GrDirectContext whose GrGLInterface has
//    glDeleteFramebuffers hooked, so we can observe the real GL bindings at the moment of deletion.
//      - GLFBODeletion_Workaround forces the workaround on (on every GPU). Any deletion of a
//        still-bound FBO means the workaround was bypassed. This fails deterministically on any
//        GL/GLES device, so it doesn't need Imagination hardware.
//      - GLFBODeletion_NoWorkaround disables driver workarounds and only logs. On an affected
//        device it is expected to reproduce the original driver UAF.
//
// 2) GLFBODriverProbe_*: raw GL call sequences (GLES only) that isolate which driver-side pattern
//    triggers the UAF, independent of Skia's state tracking. They don't check anything; on an
//    affected device they crash. The driver isn't instrumented, so freed driver memory is most
//    reliably caught with ASAN's free-fill (ASAN_OPTIONS=max_free_fill_size=...) or with MTE.
//
// Run each test on its own (e.g. --match '^GLFBODriverProbe_DeleteBoundRead$') so that a crash can
// be attributed to a single pattern.

#include "tests/Test.h"

#ifdef SK_GL

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
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

#include <memory>
#include <vector>

namespace {

// GL_READ_FRAMEBUFFER_BINDING (GLES 3.0 / GL 3.0). GR_GL_FRAMEBUFFER_BINDING has the same value as
// GL_DRAW_FRAMEBUFFER_BINDING.
constexpr GrGLenum kReadFramebufferBinding = 0x8CAA;

constexpr int kSkiaIterations = 32;
constexpr int kProbeIterations = 256;
constexpr int kProbeSize = 64;

bool supports_read_draw_fbo_targets(const GrGLInterface* gl) {
    return GrGLGetVersion(gl) >= GR_GL_VER(3, 0) && gl->fFunctions.fBlitFramebuffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Part 1: Skia-level scenarios with an observed glDeleteFramebuffers.
////////////////////////////////////////////////////////////////////////////////////////////////////

// Records the real GL framebuffer bindings at the time of every glDeleteFramebuffers call.
class FBODeletionWatcher {
public:
    struct Deletion {
        GrGLuint fFBO;
        bool fBoundDraw;
        bool fBoundRead;
    };

    // Returns a copy of 'base' that routes glDeleteFramebuffers through this watcher. The watcher
    // must outlive any GrDirectContext created with the returned interface.
    sk_sp<const GrGLInterface> wrap(sk_sp<const GrGLInterface> base) {
        fBase = std::move(base);
        fCanQueryRead = supports_read_draw_fbo_targets(fBase.get());
        sk_sp<GrGLInterface> hooked(new GrGLInterface(*fBase));
        FBODeletionWatcher* self = this;
        hooked->fFunctions.fDeleteFramebuffers = [self](GrGLsizei n, const GrGLuint* fbos) {
            self->onDeleteFramebuffers(n, fbos);
        };
        return hooked;
    }

    void reset() { fDeletions.clear(); }

    const std::vector<Deletion>& deletions() const { return fDeletions; }

private:
    void onDeleteFramebuffers(GrGLsizei n, const GrGLuint* fbos) {
        GrGLint draw = 0;
        GrGLint read = 0;
        fBase->fFunctions.fGetIntegerv(GR_GL_FRAMEBUFFER_BINDING, &draw);
        if (fCanQueryRead) {
            fBase->fFunctions.fGetIntegerv(kReadFramebufferBinding, &read);
        } else {
            read = draw;  // Without separate targets GL_FRAMEBUFFER is both.
        }
        for (GrGLsizei i = 0; i < n; ++i) {
            fDeletions.push_back({fbos[i],
                                  fbos[i] == static_cast<GrGLuint>(draw),
                                  fbos[i] == static_cast<GrGLuint>(read)});
        }
        fBase->fFunctions.fDeleteFramebuffers(n, fbos);
    }

    sk_sp<const GrGLInterface> fBase;
    bool fCanQueryRead = false;
    std::vector<Deletion> fDeletions;
};

const SkImageInfo kSurfaceInfo =
        SkImageInfo::Make(128, 128, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

void draw_content(SkCanvas* canvas, int i) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setColor((i & 1) ? SK_ColorRED : SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(8 + (i % 16), 8, 64, 64), paint);
}

// The originally reported pattern: Skia deletes the FBO it last drew to. Skia's shadow state is
// accurate here, so with the workaround enabled FBO 0 gets bound before the delete.
bool scenario_delete_last_drawn(GrDirectContext* dContext) {
    for (int i = 0; i < kSkiaIterations; ++i) {
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, kSurfaceInfo);
        if (!surface) {
            return false;
        }
        draw_content(surface->getCanvas(), i);
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        surface.reset();
        // Purges the render target: GrGLRenderTarget::onRelease() -> GrGLGpu::deleteFramebuffer()
        // while its FBO is still the bound draw framebuffer.
        dContext->freeGpuResources();
    }
    return true;
}

// Same as above, except that the client calls resetContext() between the flush and the delete
// without touching any framebuffer bindings (e.g. Chrome's PessimisticallyResetGrContext(), or a
// client that made unrelated GL calls). GrGLGpu::onResetContext() sets fBoundDrawFramebuffer to 0
// rather than "unknown", and deleteFramebuffer() calls handleDirtyContext() before its check, so
// the workaround is skipped even though the FBO is still bound in GL.
bool scenario_delete_after_reset_context(GrDirectContext* dContext) {
    for (int i = 0; i < kSkiaIterations; ++i) {
        sk_sp<SkSurface> surface =
                SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, kSurfaceInfo);
        if (!surface) {
            return false;
        }
        draw_content(surface->getCanvas(), i);
        dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
        dContext->resetContext();
        surface.reset();
        dContext->freeGpuResources();
    }
    return true;
}

// A scaling copy into a non-renderable texture goes through copySurfaceAsBlitFramebuffer(), which
// leaves the source FBO bound to GL_READ_FRAMEBUFFER (and the temp dst FBO bound to
// GL_DRAW_FRAMEBUFFER). Skia only tracks the draw binding, so deleting the source afterwards skips
// the workaround. MSAA (non-MSRTT) snapshot copies end up in the same state.
bool scenario_delete_blit_source(GrDirectContext* dContext) {
    auto gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());
    if (!supports_read_draw_fbo_targets(gpu->glInterface())) {
        return false;
    }

    const GrBackendFormat format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
    const SkIRect srcRect = SkIRect::MakeWH(64, 64);
    const SkIRect dstRect = SkIRect::MakeWH(32, 32);
    const GrTextureType texType = GrTextureType::k2D;
    // A renderable dst would be copied as a draw, so the dst must be non-renderable. Then the only
    // way GrGLGpu::onCopySurface() can do a scaling copy is glBlitFramebuffer.
    if (!gpu->glCaps().canCopyAsBlit(GrGLFormat::kRGBA8, /*dstSampleCnt=*/0, &texType,
                                     GrGLFormat::kRGBA8, /*srcSampleCnt=*/1, &texType,
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
    for (int i = 0; i < kSkiaIterations; ++i) {
        sk_sp<GrTexture> src = gpu->createTexture(srcRect.size(), format, texType,
                                                  GrRenderable::kYes, 1, skgpu::Mipmapped::kNo,
                                                  skgpu::Budgeted::kNo, GrProtected::kNo,
                                                  "GLFBODeletion_BlitSrc");
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

struct SkiaScenario {
    const char* fName;
    bool (*fRun)(GrDirectContext*);
    // Whether the workaround, as currently implemented, is expected to cover this scenario.
    bool fCoveredByCurrentWorkaround;
};

constexpr SkiaScenario kSkiaScenarios[] = {
        {"DeleteLastDrawn", scenario_delete_last_drawn, true},
        {"DeleteAfterResetContext", scenario_delete_after_reset_context, false},
        {"DeleteBlitSource", scenario_delete_blit_source, false},
};

enum class WorkaroundMode { kForceOn, kAllDisabled };

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
        FBODeletionWatcher watcher;
        bool ran = false;
        {
            sk_sp<GrDirectContext> dContext =
                    GrDirectContexts::MakeGL(watcher.wrap(sk_ref_sp(glCtx->gl())), options);
            if (!dContext) {
                ERRORF(reporter, "Could not create a GrDirectContext with the hooked interface");
                return;
            }
            auto gpu = static_cast<GrGLGpu*>(dContext->priv().getGpu());
            if (mode == WorkaroundMode::kForceOn &&
                !gpu->glCaps().workarounds().ensure_previous_framebuffer_not_deleted) {
                ERRORF(reporter, "ensure_previous_framebuffer_not_deleted did not get enabled");
                return;
            }
            // Ignore the deletions made while creating the context (e.g. stencil format probing).
            watcher.reset();
            ran = scenario.fRun(dContext.get());
            // Destroying the context deletes the temp src/dst FBOs, which can be bound as well.
        }

        if (!ran) {
            INFOF(reporter, "%s: skipped (unsupported)\n", scenario.fName);
            continue;
        }

        int boundDraw = 0;
        int boundRead = 0;
        for (const FBODeletionWatcher::Deletion& d : watcher.deletions()) {
            boundDraw += d.fBoundDraw ? 1 : 0;
            boundRead += (d.fBoundRead && !d.fBoundDraw) ? 1 : 0;
        }
        const int total = static_cast<int>(watcher.deletions().size());
        INFOF(reporter, "%s: %d FBO deletions, %d while bound for draw, %d while bound for read\n",
              scenario.fName, total, boundDraw, boundRead);

        if (mode == WorkaroundMode::kForceOn && (boundDraw || boundRead)) {
            ERRORF(reporter,
                   "%s: ensure_previous_framebuffer_not_deleted is enabled, but %d of %d FBO "
                   "deletions happened while the FBO was still bound (%d draw, %d read-only). "
                   "%s",
                   scenario.fName, boundDraw + boundRead, total, boundDraw, boundRead,
                   scenario.fCoveredByCurrentWorkaround
                           ? "This scenario is supposed to be covered by the current workaround."
                           : "The current workaround does not cover this case.");
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
        if (fScratchFBO) {
            this->deleteFBO(fScratchFBO);
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

        fScratchTex = this->makeTexture();
        fScratchFBO = this->makeFBO(fScratchTex);
        return fScratchFBO != 0;
    }

    GrGLuint makeTexture() {
        GrGLuint tex = 0;
        GR_GL_CALL(fGL, GenTextures(1, &tex));
        GR_GL_CALL(fGL, BindTexture(GR_GL_TEXTURE_2D, tex));
        GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
        GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
        GR_GL_CALL(fGL, TexImage2D(GR_GL_TEXTURE_2D, 0, GR_GL_RGBA, kProbeSize, kProbeSize, 0,
                                   GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, nullptr));
        GR_GL_CALL(fGL, BindTexture(GR_GL_TEXTURE_2D, 0));
        return tex;
    }

    // Leaves the new FBO bound to GL_FRAMEBUFFER.
    GrGLuint makeFBO(GrGLuint tex) {
        GrGLuint fbo = 0;
        GR_GL_CALL(fGL, GenFramebuffers(1, &fbo));
        GR_GL_CALL(fGL, BindFramebuffer(GR_GL_FRAMEBUFFER, fbo));
        GR_GL_CALL(fGL, FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                             GR_GL_TEXTURE_2D, tex, 0));
        GrGLenum status;
        GR_GL_CALL_RET(fGL, status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            this->deleteFBO(fbo);
            return 0;
        }
        return fbo;
    }

    void bindFBO(GrGLenum target, GrGLuint fbo) {
        GR_GL_CALL(fGL, BindFramebuffer(target, fbo));
    }

    void deleteFBO(GrGLuint fbo) { GR_GL_CALL(fGL, DeleteFramebuffers(1, &fbo)); }

    void deleteTexture(GrGLuint tex) { GR_GL_CALL(fGL, DeleteTextures(1, &tex)); }

    // Binds 'fbo' to GL_FRAMEBUFFER and records a clear plus a draw into it, so that the driver has
    // pending (tiled) work referencing the framebuffer. Leaves 'fbo' bound.
    void drawInto(GrGLuint fbo, int i) {
        this->bindFBO(GR_GL_FRAMEBUFFER, fbo);
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

    // Blits 'src' (bound to GL_READ_FRAMEBUFFER) into 'dst' (bound to GL_DRAW_FRAMEBUFFER) and
    // leaves both bindings in place.
    void blit(GrGLuint src, GrGLuint dst) {
        this->bindFBO(GR_GL_READ_FRAMEBUFFER, src);
        this->bindFBO(GR_GL_DRAW_FRAMEBUFFER, dst);
        GR_GL_CALL(fGL, BlitFramebuffer(0, 0, kProbeSize, kProbeSize, 0, 0, kProbeSize / 2,
                                        kProbeSize / 2, GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
    }

    // Allocates and frees a few driver objects so freed framebuffer memory is likely to be reused,
    // then does work that makes the driver look at its framebuffer state (bind changes, a draw,
    // and a flush/finish/readback).
    void pokeDriver(int i) {
        GrGLuint texs[4];
        GrGLuint fbos[4];
        for (int j = 0; j < 4; ++j) {
            texs[j] = this->makeTexture();
            fbos[j] = this->makeFBO(texs[j]);
        }
        this->bindFBO(GR_GL_FRAMEBUFFER, 0);
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

    const GrGLInterface* gl() const { return fGL; }

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
    GrGLuint fScratchTex = 0;
    GrGLuint fScratchFBO = 0;
};

// The original bug with no workaround: delete the FBO while it is bound for drawing.
void probe_delete_bound_draw(RawGL& gl, sk_gpu_test::GLTestContext*, int i) {
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.drawInto(fbo, i);
    gl.deleteFBO(fbo);  // Still bound to GL_FRAMEBUFFER.
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// What the shipped workaround does: bind FBO 0, then delete. If this crashes, binding 0 is not
// enough to make the driver drop its reference.
void probe_bind_zero_then_delete(RawGL& gl, sk_gpu_test::GLTestContext*, int i) {
    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.drawInto(fbo, i);
    gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(fbo);
    gl.pokeDriver(i);
    gl.deleteTexture(tex);
}

// Delete an FBO that is only bound to GL_READ_FRAMEBUFFER. Skia doesn't track the read binding.
void probe_delete_bound_read(RawGL& gl, sk_gpu_test::GLTestContext*, int i) {
    GrGLuint srcTex = gl.makeTexture();
    GrGLuint src = gl.makeFBO(srcTex);
    GrGLuint dstTex = gl.makeTexture();
    GrGLuint dst = gl.makeFBO(dstTex);
    gl.drawInto(src, i);
    gl.drawInto(dst, i + 1);
    gl.blit(src, dst);
    gl.deleteFBO(src);  // Bound to GL_READ_FRAMEBUFFER only.
    gl.pokeDriver(i);
    gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(dst);
    gl.deleteTexture(srcTex);
    gl.deleteTexture(dstTex);
}

// Delete the FBO that was bound *before* the current one. The workaround only looks at the
// currently bound FBO, so if the driver's stale reference is to the previous binding this crashes.
void probe_delete_previously_bound(RawGL& gl, sk_gpu_test::GLTestContext*, int i) {
    GrGLuint texA = gl.makeTexture();
    GrGLuint a = gl.makeFBO(texA);
    GrGLuint texB = gl.makeTexture();
    GrGLuint b = gl.makeFBO(texB);
    gl.drawInto(a, i);
    gl.drawInto(b, i + 1);
    gl.deleteFBO(a);  // Not bound, but it was the previous binding.
    gl.pokeDriver(i);
    gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(b);
    gl.deleteTexture(texA);
    gl.deleteTexture(texB);
}

// Like probe_bind_zero_then_delete(), but with eglMakeCurrent() switches between drawing,
// deleting, and the follow-up work (ANGLE has a similar Imagination workaround:
// unbindFBOBeforeSwitchingContext). Needs a second context.
void probe_make_current_switch(RawGL& gl, sk_gpu_test::GLTestContext* ctx, int i) {
    static thread_local std::unique_ptr<sk_gpu_test::GLTestContext> sOtherCtx;
    static thread_local std::unique_ptr<RawGL> sOtherGL;
    if (i == 0) {
        sOtherGL.reset();
        sOtherCtx = ctx->makeNew();  // Makes the new context current.
        if (sOtherCtx) {
            sOtherGL = std::make_unique<RawGL>(sOtherCtx->gl());
            if (!sOtherGL->init()) {
                sOtherGL.reset();
            }
        }
        ctx->makeCurrent();
    }
    if (!sOtherGL) {
        return;
    }
    auto switchAndWork = [&] {
        sOtherCtx->makeCurrent();
        sOtherGL->pokeDriver(i);
        ctx->makeCurrent();
    };

    GrGLuint tex = gl.makeTexture();
    GrGLuint fbo = gl.makeFBO(tex);
    gl.drawInto(fbo, i);
    switchAndWork();
    gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
    gl.deleteFBO(fbo);
    switchAndWork();
    gl.pokeDriver(i);
    gl.deleteTexture(tex);

    if (i == kProbeIterations - 1) {
        sOtherCtx->makeCurrent();
        sOtherGL.reset();
        sOtherCtx.reset();
        ctx->makeCurrent();
    }
}

using ProbeFn = void (*)(RawGL&, sk_gpu_test::GLTestContext*, int);

void run_driver_probe(skiatest::Reporter* reporter,
                      const sk_gpu_test::ContextInfo& ctxInfo,
                      ProbeFn probe,
                      bool needsReadDrawTargets) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    const GrGLInterface* gli = glCtx->gl();
    if (gli->fStandard != kGLES_GrGLStandard) {
        INFOF(reporter, "Driver probes only run on GLES\n");
        return;
    }
    if (needsReadDrawTargets && !supports_read_draw_fbo_targets(gli)) {
        INFOF(reporter, "Probe needs separate read/draw framebuffer targets\n");
        return;
    }
    ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);
    {
        RawGL gl(gli);
        if (!gl.init()) {
            ERRORF(reporter, "Failed to set up raw GL state");
            return;
        }
        for (int i = 0; i < kProbeIterations; ++i) {
            probe(gl, glCtx, i);
        }
        gl.bindFBO(GR_GL_FRAMEBUFFER, 0);
        GR_GL_CALL(gli, Finish());
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

#define DEF_FBO_DRIVER_PROBE(name, probe, needsReadDrawTargets)       \
    DEF_GANESH_TEST_FOR_GL_CONTEXT(GLFBODriverProbe_##name,           \
                                   reporter,                          \
                                   ctxInfo,                           \
                                   CtsEnforcement::kNever) {          \
        run_driver_probe(reporter, ctxInfo, probe, needsReadDrawTargets); \
    }

DEF_FBO_DRIVER_PROBE(DeleteBoundDraw, probe_delete_bound_draw, false)
DEF_FBO_DRIVER_PROBE(BindZeroThenDelete, probe_bind_zero_then_delete, false)
DEF_FBO_DRIVER_PROBE(DeleteBoundRead, probe_delete_bound_read, true)
DEF_FBO_DRIVER_PROBE(DeletePreviouslyBound, probe_delete_previously_bound, false)
DEF_FBO_DRIVER_PROBE(MakeCurrentSwitch, probe_make_current_switch, false)

#undef DEF_FBO_DRIVER_PROBE

#endif  // SK_GL
