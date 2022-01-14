
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkTHash.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/sdl2/Window_sdl2.h"
#include "tools/timer/Timer.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <SDL2/SDL.h>
#include <unordered_set>

using sk_app::Window_sdl2;

static bool gQuit = false;

static void main_loop_iter(void* arg) {
    auto app = reinterpret_cast<sk_app::Application*>(arg);
    if (gQuit) {
        // How or do we ever stop in emscripten?
        return;
    }

    SDL_PumpEvents();
    static constexpr int kMaxEvents = 32;
    SDL_Event events[kMaxEvents];
    int n = SDL_PeepEvents(events, kMaxEvents, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
    if (n < 0) {
        SkDebugf("Error peeping events: %s\n", SDL_GetError());
        return; // keep going, I guess?
    }
    // We collapse multiple repaint and resize messages from the queue. Should we track which
    // was last if win was resized and repainted?
    std::unordered_set<Uint32> repaintWindowIds;
    std::unordered_set<Uint32> resizeWindowIds;
    for (int i = 0; i < n; ++i) {
        const SDL_Event& event = events[i];
        switch (event.type) {
            case SDL_QUIT:
                gQuit = true;
                return;
            case SDL_WINDOWEVENT: {
                SkDebugf("\twindow.event %x\n", event.window.event);
                Uint32 id = event.window.windowID;
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_EXPOSED:
                        repaintWindowIds.insert(id);
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resizeWindowIds.insert(id);
                        break;
                    default:
                        if (auto win = Window_sdl2::GetWindowFromID(id)) {
                            win->handleWindowEvent(event.window);
                        }
                        break;
                }
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (auto win = Window_sdl2::GetWindowFromID(event.key.windowID)) {
                    win->handleKeyEvent(event.key);
                }
                break;
            case SDL_TEXTINPUT:
                if (auto win = Window_sdl2::GetWindowFromID(event.text.windowID)) {
                    win->handleText(event.text.text);
                }
                break;
            case SDL_MOUSEMOTION:
                if (auto win = Window_sdl2::GetWindowFromID(event.motion.windowID)) {
                    win->handleMouseMotionEvent(event.motion);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (auto win = Window_sdl2::GetWindowFromID(event.motion.windowID)) {
                    win->handleMouseButtonEvent(event.button);
                }
                break;
            case SDL_MOUSEWHEEL:
                if (auto win = Window_sdl2::GetWindowFromID(event.motion.windowID)) {
                    win->handleMouseWheelEvent(event.wheel);
                }
                break;
        }
        if (!Window_sdl2::NumWindows()) {
            gQuit = true;
            return;
        }
    }
    for (Uint32 id : resizeWindowIds) {
        if (Window_sdl2* win = Window_sdl2::GetWindowFromID(id)) {
            int w, h;
            SDL_GetWindowSize(win->sdlWindow(), &w, &h);
            win->onResize(w, h);
        }
    }
    for (Uint32 id : repaintWindowIds) {
        if (Window_sdl2* win = Window_sdl2::GetWindowFromID(id)) {
            win->onPaint();
        }
    }
    app->onIdle();
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        SkDebugf("Failed to init SDL: %s\n", SDL_GetError());
        return -1;
    }

    sk_app::Application* app = sk_app::Application::Create(argc, argv, (void*)nullptr);

    SDL_StartTextInput();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop_iter, (void*) app, 0, 1);
#else
    while(!gQuit) {
        main_loop_iter(app);
    }
#endif

    delete app;
    SDL_Quit();
    return 0;
}
