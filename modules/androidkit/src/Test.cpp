/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <android/log.h>
#include <chrono>
#include <jni.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "modules/androidkit/src/Utils.h"
#include "modules/skottie/include/Skottie.h"

namespace {

class GLSurface {
public:
    GLSurface(int width, int height) {
        fEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (fEGLDisplay == EGL_NO_DISPLAY) {
            return;
        }

        EGLint majVer, minVer;
        if (!eglInitialize(fEGLDisplay, &majVer, &minVer) ||
            !eglBindAPI(EGL_OPENGL_ES_API)) {
            return;
        }

        EGLint eglNumConfigs = 0,
               eglSampleCnt  = 0; // TODO: MSAA?

        const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_STENCIL_SIZE, 8,
            EGL_SAMPLE_BUFFERS, eglSampleCnt ? 1 : 0,
            EGL_SAMPLES, eglSampleCnt,
            EGL_NONE
        };

        EGLConfig surfaceConfig;
        if (!eglChooseConfig(fEGLDisplay, configAttribs, &surfaceConfig, 1, &eglNumConfigs) ||
            eglNumConfigs <= 0) {
            return;
        }

        static const EGLint kEGLContextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        fEGLContext = eglCreateContext(fEGLDisplay, surfaceConfig, nullptr, kEGLContextAttribs);
        if (fEGLContext == EGL_NO_CONTEXT) {
            return;
        }

        static const EGLint kEGLPbufferAttribs[] = {
            EGL_WIDTH , width,
            EGL_HEIGHT, height,
            EGL_NONE
        };
        fEGLSurface = eglCreatePbufferSurface(fEGLDisplay, surfaceConfig, kEGLPbufferAttribs);
        if (fEGLSurface == EGL_NO_SURFACE) {
            return;
        }

        if (!eglMakeCurrent(fEGLDisplay, fEGLSurface, fEGLSurface, fEGLContext)) {
            return;
        }

        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        GrContextOptions options;
        fGrContext = GrDirectContext::MakeGL(GrGLMakeNativeInterface(), options);
        if (!fGrContext) {
            return;
        }

        GrGLFramebufferInfo fboInfo;
        fboInfo.fFBOID = 0;
        fboInfo.fFormat = GL_RGBA8;

        GrBackendRenderTarget backendRT(width, height, 0, 8, fboInfo);
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        fSurface = SkSurface::MakeFromBackendRenderTarget(fGrContext.get(), backendRT,
                                                          kBottomLeft_GrSurfaceOrigin,
                                                          kN32_SkColorType,
                                                          nullptr, &props);

        __android_log_print(ANDROID_LOG_INFO, "GLSurface", "EGL %d.%d surface initialized: %p",
                            majVer, minVer, this);
    }

    ~GLSurface() {
        fSurface.reset();
        fGrContext.reset();

        SkAssertResult(eglMakeCurrent(fEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        if (fEGLSurface != EGL_NO_SURFACE) {
            SkAssertResult(eglDestroySurface(fEGLDisplay, fEGLSurface));
        }
        if (fEGLContext != EGL_NO_CONTEXT) {
            SkAssertResult(eglDestroyContext(fEGLDisplay, fEGLContext));
        }
        if (fEGLDisplay != EGL_NO_DISPLAY) {
            SkAssertResult(eglTerminate(fEGLDisplay));
        }

        __android_log_print(ANDROID_LOG_INFO, "GLSurface", "EGL surface destroyed: %p", this);
    }

    SkSurface* operator->() const { return fSurface.get(); }

    operator bool() const { return !!fSurface; }

private:
    EGLDisplay                 fEGLDisplay = EGL_NO_DISPLAY;
    EGLContext                 fEGLContext = EGL_NO_CONTEXT;
    EGLSurface                 fEGLSurface = EGL_NO_SURFACE;
    sk_sp<GrDirectContext>     fGrContext;
    sk_sp<SkSurface>           fSurface;
};

static void GpuDecode(JNIEnv* env, jobject, jstring jstr, jint width, jint height, float fps) {
    GLSurface surface(width, height);
    if (!surface) {
        __android_log_print(ANDROID_LOG_ERROR, "GpuDecode", "!! failed to initialize a GL surface");
        return;
    }

    androidkit::utils::CString json(env, jstr);
    const char* json_cstr = json;
    auto anim = skottie::Animation::Make(json_cstr, strlen(json_cstr));
    SkASSERT(anim);

    const auto frame_count = static_cast<int>(anim->duration() * fps);

    __android_log_print(ANDROID_LOG_INFO, "GpuDecode", "Rendering %d frames", frame_count);
    const auto dest = SkRect::MakeIWH(width, height);

    auto image_info = SkImageInfo::MakeN32Premul(width, height);

    auto read_cb = [](SkSurface::ReadPixelsContext ctx,
                      std::unique_ptr<const SkSurface::AsyncReadResult> result) {
        // pass data to encoder
    };

    const auto t_start = std::chrono::steady_clock::now();

    for (int f = 0; f < frame_count; ++f) {
        anim->seekFrameTime(f/fps);
        anim->render(surface->getCanvas(), &dest);

        surface->asyncRescaleAndReadPixels(image_info, SkIRect::MakeWH(width, height),
                                           SkSurface::RescaleGamma::kSrc,
                                           SkImage::RescaleMode::kNearest,
                                           read_cb, nullptr);
        // submit frame + readback request to GPU
        surface->flushAndSubmit();
    }

    // wait for all pending reads to complete
    surface->flushAndSubmit(true);

    const auto t_stop = std::chrono::steady_clock::now();
    __android_log_print(ANDROID_LOG_INFO, "GpuDecode", "Ellapsed ms: %d",
                        duration_cast<std::chrono::milliseconds>(t_stop - t_start).count());
}

} // namespace

int register_androidkit_Test(JNIEnv* env) {
    static const JNINativeMethod methods[] = {
        {"GpuDecode", "(Ljava/lang/String;IIF)V", reinterpret_cast<void*>(GpuDecode)},
    };

    const auto clazz = env->FindClass("org/skia/androidkit/Canvas");
    return clazz
        ? env->RegisterNatives(clazz, methods, SK_ARRAY_COUNT(methods))
        : JNI_ERR;
}
