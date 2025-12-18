
/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#include "include/gpu/ganesh/gl/GrGLMakeWebGLInterface.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <emscripten/html5.h>
#include <GLES3/gl3.h>

namespace {

std::function<void()> context_restorer() {
    auto context = emscripten_webgl_get_current_context();
    return [context] { emscripten_webgl_make_context_current(context); };
}

// On the web, threads run in web workers. OffscreenCanvas is the interface used for
// rendering inside web workers.
EM_JS(intptr_t, createOffscreenContextJs, (int width, int height), {
    const offscreenCanvas = new OffscreenCanvas(width, height);
    const contextAttributes = {
        majorVersion : 2,
        minorVersion : 0,
    };
    const contextHandle = GL.createContext(offscreenCanvas, contextAttributes);
    return contextHandle;
})

class WebGLTestContext : public sk_gpu_test::GLTestContext {
public:
    WebGLTestContext(WebGLTestContext* shareContext);
    ~WebGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE fContext;
};

WebGLTestContext::WebGLTestContext(WebGLTestContext* shareContext)
    : fContext(0) {
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.majorVersion = 2;
    attributes.minorVersion = 0;
    attributes.failIfMajorPerformanceCaveat = true;
    attributes.stencil = true;

    // fContext = emscripten_webgl_create_context("#canvas", &attributes);
    fContext = createOffscreenContextJs(1024, 1024);
    if (fContext == 0) {
        SkDebugf("emscripten_webgl_create_context failed.\n");
        return;
    }

    SkScopeExit restorer(context_restorer());
    if (EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(fContext);
        res != EMSCRIPTEN_RESULT_SUCCESS) {
        SkDebugf("emscripten_webgl_make_context_current failed: %d.\n", res);
        return;
    }

#if defined(GPU_TEST_UTILS)
    if (!emscripten_webgl_enable_extension(fContext, "WEBGL_debug_renderer_info")) {
        SkDebugf("warning: WEBGL_debug_renderer_info extension not available.\n");
    }
#endif

    auto gl = GrGLInterfaces::MakeWebGL();
    if (!gl) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Context could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }

    this->init(std::move(gl));
}

WebGLTestContext::~WebGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void WebGLTestContext::destroyGLContext() {
    if (fContext) {
        if (emscripten_webgl_get_current_context() == fContext) {
            if (auto res = emscripten_webgl_make_context_current(0); res != EMSCRIPTEN_RESULT_SUCCESS) {
                SkDebugf("destroyGLContext: emscripten_webgl_make_context_current failed: %d\n", res);
            }
        }
        if (auto res = emscripten_webgl_destroy_context(fContext); res != EMSCRIPTEN_RESULT_SUCCESS) {
            SkDebugf("destroyGLContext: emscripten_webgl_destroy_context failed: %d\n", res);
        }
        fContext = 0;
    }
}

void WebGLTestContext::onPlatformMakeNotCurrent() const {
    if (auto res = emscripten_webgl_make_context_current(0); res != EMSCRIPTEN_RESULT_SUCCESS) {
        SkDebugf("onPlatformMakeNotCurrent: emscripten_webgl_make_context_current failed: %d\n", res);
    }
}

void WebGLTestContext::onPlatformMakeCurrent() const {
    if (auto res = emscripten_webgl_make_context_current(fContext); res != EMSCRIPTEN_RESULT_SUCCESS) {
        SkDebugf("onPlatformMakeCurrent: emscripten_webgl_make_context_current failed: %d\n", res);
    }
}

std::function<void()> WebGLTestContext::onPlatformGetAutoContextRestore() const {
    if (emscripten_webgl_get_current_context() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr WebGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    return nullptr;
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext* shareContext) {
    // if (forcedGpuAPI != kNone_GrGLStandard && forcedGpuAPI != kWebGL_GrGLStandard) {
    //     SkDebugf("CreatePlatformGLTestContext: cannot force gpu api %d\n", static_cast<int>(forcedGpuAPI));
    //     return nullptr;
    // }

    WebGLTestContext* webglShareContext = reinterpret_cast<WebGLTestContext*>(shareContext);
    WebGLTestContext *ctx = new WebGLTestContext(webglShareContext);
    if (!ctx->isValid()) {
        SkDebugf("CreatePlatformGLTestContext: webgl context validation failed\n");
        delete ctx;
        return nullptr;
    }

    return ctx;
}
}  // namespace sk_gpu_test
