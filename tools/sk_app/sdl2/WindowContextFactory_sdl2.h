
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef WindowContextFactory_sdl2_DEFINED
#define WindowContextFactory_sdl2_DEFINED
#include "tools/sk_app/WindowContext.h"

#include "SDL.h"
#include <memory>
namespace sk_app {
class WindowContext;
struct DisplayParams;
namespace window_context_factory {
struct SDL2WindowInfo {
    SDL_Window* window;
};
#ifdef SK_VULKAN
std::unique_ptr<WindowContext> MakeVulkanForSDL2(const SDL2WindowInfo&, const DisplayParams&);
#endif
std::unique_ptr<WindowContext> MakeGLForSDL2(const SDL2WindowInfo&, const DisplayParams&);
#ifdef SK_DAWN
// TODO
// std::unique_ptr<WindowContext> MakeDawnVulkanForSDL2(const SDLWindowInfo&, const DisplayParams&);
#endif
#ifdef SK_METAL
// TODO
// std::unique_ptr<WindowContext> MakeMetalForSDL2(const SDLWindowInfo&, const DisplayParams&);
#endif
std::unique_ptr<WindowContext> MakeRasterForSDL2(const SDL2WindowInfo&, const DisplayParams&);
}  // namespace window_context_factory
}  // namespace sk_app
#endif
