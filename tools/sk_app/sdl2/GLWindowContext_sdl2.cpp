
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/sdl2/WindowContextFactory_sdl2.h"
using sk_app::DisplayParams;
using sk_app::GLWindowContext;
using sk_app::window_context_factory::SDL2WindowInfo;
namespace {
class GLWindowContext_sdl2 : public GLWindowContext {
public:
    GLWindowContext_sdl2(const SDL2WindowInfo&, const DisplayParams&);
    ~GLWindowContext_sdl2() override;
    void onSwapBuffers() override;
    void onDestroyContext() override;
protected:
    sk_sp<const GrGLInterface> onInitializeContext() override;
private:
    SDL_Window* fWindow = nullptr;
    SDL_GLContext fGLContext = nullptr;
    using INHERITED = GLWindowContext;
};
GLWindowContext_sdl2::GLWindowContext_sdl2(const SDL2WindowInfo& winInfo,
                                           const DisplayParams& params)
        : INHERITED(params), fWindow(winInfo.window) {
    this->initializeContext();
}
sk_sp<const GrGLInterface> GLWindowContext_sdl2::onInitializeContext() {
    fGLContext = SDL_GL_CreateContext(fWindow);
    if (!fGLContext) {
        return nullptr;
    }
    SDL_GL_MakeCurrent(fWindow, fGLContext);
    auto getProc = [](void*, const char* name) -> GrGLFuncPtr {
        if (0 == strcmp("eglQueryString", name)) {
            return nullptr;
        }
        return reinterpret_cast<GrGLFuncPtr>(SDL_GL_GetProcAddress(name));
    };
    auto interface = GrGLMakeAssembledInterface(nullptr, getProc);
    if (!interface) {
        SDL_GL_DeleteContext(fGLContext);
        fGLContext = nullptr;
        return nullptr;
    }

    // SDL_GL_GetDrawableSize(fWindow, &fWidth, &fHeight);
    return interface;
}
GLWindowContext_sdl2::~GLWindowContext_sdl2() { this->destroyContext(); }
void GLWindowContext_sdl2::onDestroyContext() {
    if (fGLContext) {
        SDL_GL_DeleteContext(fGLContext);
        fGLContext = nullptr;
    }
}
void GLWindowContext_sdl2::onSwapBuffers() { SDL_GL_SwapWindow(fWindow); }
}  // anonymous namespace
namespace sk_app {
namespace window_context_factory {
std::unique_ptr<WindowContext> MakeGLForSDL2(const SDL2WindowInfo& winInfo,
                                             const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_sdl2(winInfo, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}
}  // namespace window_context_factory
}  // namespace sk_app
