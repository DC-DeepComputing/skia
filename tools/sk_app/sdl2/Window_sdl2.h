/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Window_sdl2_DEFINED
#define Window_sdl2_DEFINED

#include "tools/sk_app/Window.h"

#include "SDL.h"
#include <unordered_map>

namespace sk_app {

class Window_sdl2 : public Window {
public:
    Window_sdl2();

    ~Window_sdl2() override;

    static Window_sdl2* GetWindowFromID(Uint32 id);

    static int NumWindows();

    struct InitWindowState {
        SkISize     size   = {1280, 960};
        SkIPoint    pos    = {SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED};
        bool        hidden = true;
        const char* title  = "";
    };

    bool initWindow(const InitWindowState&);

    bool defaultInitWindow() { return initWindow({}); }

    void setTitle(const char*) override;

    void show() override;

    const char* getClipboardText() override;

    void setClipboardText(const char*) override;

    bool attach(BackendType) override;

    void onInval() override;

    void handleWindowEvent(const SDL_WindowEvent&);

    void handleKeyEvent(const SDL_KeyboardEvent&);

    void handleMouseButtonEvent(const SDL_MouseButtonEvent&);

    void handleMouseMotionEvent(const SDL_MouseMotionEvent&);

    void handleMouseWheelEvent(const SDL_MouseWheelEvent&);

    void handleText(const char*);

    void setRequestedDisplayParams(const DisplayParams&, bool allowReattach) override;

    SDL_Window* sdlWindow() { return fWindow; }

private:
    void closeWindow();

    SDL_Window* fWindow = nullptr;

    int fMSAASampleCount = 1;

    BackendType fBackend = BackendType::kRaster_BackendType;

    class SDLAutoClipboardTest {
    public:
        ~SDLAutoClipboardTest() { this->reset(); }

        char* retrieve() {
            this->reset();
            fText = SDL_GetClipboardText();
            return fText;
        }

    private:
        void reset() {
            if (fText) {
                SDL_free(fText);
                fText = nullptr;
            }
        }

        char* fText = nullptr;
    };
    SDLAutoClipboardTest fClipboardText;

    static std::unordered_map<Uint32, Window_sdl2*> gMap;
    using INHERITED = Window;
};

}  // namespace sk_app

#endif
