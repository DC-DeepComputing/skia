
/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/sk_app/sdl2/Window_sdl2.h"

#include "src/base/SkUTF.h"
#include "tools/sk_app/sdl2/WindowContextFactory_sdl2.h"
#include "tools/skui/ModifierKey.h"

namespace sk_app {

Window* Window::CreateNativeWindow(void* platformData) {
    Window_sdl2* window = new Window_sdl2();
    if (!window->defaultInitWindow()) {
        delete window;
        return nullptr;
    }

    return window;
}

std::unordered_map<Uint32, Window_sdl2*> Window_sdl2::gMap;

Window_sdl2* Window_sdl2::GetWindowFromID(Uint32 id) {
    if (auto iter = gMap.find(id); iter != gMap.end()) {
        return iter->second;
    }
    return nullptr;
}

int Window_sdl2::NumWindows() { return SkToInt(gMap.size()); }

Window_sdl2::Window_sdl2() {}

Window_sdl2::~Window_sdl2() { this->closeWindow(); }

bool Window_sdl2::initWindow(const InitWindowState& initState) {
    if (fRequestedDisplayParams.fMSAASampleCount != fMSAASampleCount) {
        this->closeWindow();
    }
    // we already have a window
    if (fWindow) {
        return true;
    }

    Uint32 flags = SDL_WINDOW_RESIZABLE ; // | SDL_WINDOW_ALLOW_HIGHDPI;
    if (initState.hidden) {
        flags |= SDL_WINDOW_HIDDEN;
    }
    switch (fBackend) {
#ifdef SK_GL
        case kNativeGL_BackendType:
            flags |= SDL_WINDOW_OPENGL;
            break;
#endif

#if SK_ANGLE && defined(SK_BUILD_FOR_WIN)
        case kANGLE_BackendType:
            return false;
            break;
#endif

#ifdef SK_DAWN
        case kDawn_BackendType:
#ifdef SK_BUILD_FOR_UNIX
            flags |= SDL_WINDOW_VULKAN;
            break;
#else
            return false;
#endif
#endif

#ifdef SK_VULKAN
        case kVulkan_BackendType:
            flags |= SDL_WINDOW_VULKAN;
            break;
#endif

#ifdef SK_METAL
        case kMetal_BackendType:
#ifdef SK_GRAPHITE_ENABLED
        case kGraphiteMetal_BackendType:
#endif
            flags |= SDL_WINDOW_METAL;
            break;
#endif

#ifdef SK_DIRECT3D
        case kDirect3D_BackendType:
            return false;
#endif

        case kRaster_BackendType:
            break;
    }
    fWindow = SDL_CreateWindow(initState.title,
                               initState.pos.x(),
                               initState.pos.y(),
                               initState.size.width(),
                               initState.size.height(),
                               flags);

    if (!fWindow) {
        return false;
    }

    fMSAASampleCount = fRequestedDisplayParams.fMSAASampleCount;

    Uint32 id = SDL_GetWindowID(fWindow);
    gMap[id] = this;

    return true;
}

void Window_sdl2::closeWindow() {
    if (fWindow) {
        Uint32 id = SDL_GetWindowID(fWindow);
        this->detach();
        SDL_DestroyWindow(fWindow);
        gMap.erase(id);
        fWindow = nullptr;
    }
}

void Window_sdl2::handleWindowEvent(const SDL_WindowEvent& event) {
    switch (event.event) {
        case SDL_WINDOWEVENT_CLOSE:
            this->closeWindow();
            return;
    }
}

static skui::Key get_key(SDL_Keycode keycode) {
    switch (keycode) {
        case SDLK_HOME:
        case SDLK_AC_HOME:       return skui::Key::kHome;

        case SDLK_BACKSPACE:
        case SDLK_KP_BACKSPACE:  return skui::Key::kBack;

        case SDLK_END:           return skui::Key::kEnd;
        case SDLK_0:             return skui::Key::k0;
        case SDLK_1:             return skui::Key::k1;
        case SDLK_2:             return skui::Key::k2;
        case SDLK_3:             return skui::Key::k3;
        case SDLK_4:             return skui::Key::k4;
        case SDLK_5:             return skui::Key::k5;
        case SDLK_6:             return skui::Key::k6;
        case SDLK_7:             return skui::Key::k7;
        case SDLK_8:             return skui::Key::k8;
        case SDLK_9:             return skui::Key::k9;
        case SDLK_ASTERISK:      return skui::Key::kStar;
        case SDLK_HASH:          return skui::Key::kHash;
        case SDLK_UP:            return skui::Key::kUp;
        case SDLK_DOWN:          return skui::Key::kDown;
        case SDLK_LEFT:          return skui::Key::kLeft;
        case SDLK_RIGHT:         return skui::Key::kRight;
        case SDLK_TAB:           return skui::Key::kTab;
        case SDLK_PAGEDOWN:      return skui::Key::kPageDown;
        case SDLK_PAGEUP:        return skui::Key::kPageUp;
        case SDLK_DELETE:        return skui::Key::kDelete;
        case SDLK_ESCAPE:        return skui::Key::kEscape;

        case SDLK_LSHIFT:
        case SDLK_RSHIFT:        return skui::Key::kShift;

        case SDLK_LCTRL:
        case SDLK_RCTRL:         return skui::Key::kCtrl;

        case SDLK_LALT:
        case SDLK_RALT:          return skui::Key::kOption;

        case SDLK_LGUI:
        case SDLK_RGUI:          return skui::Key::kSuper;

        case SDLK_a:             return skui::Key::kA;
        case SDLK_c:             return skui::Key::kC;
        case SDLK_v:             return skui::Key::kV;
        case SDLK_x:             return skui::Key::kX;
        case SDLK_y:             return skui::Key::kY;
        case SDLK_z:             return skui::Key::kZ;
        case SDLK_VOLUMEUP:      return skui::Key::kVolUp;
        case SDLK_VOLUMEDOWN:    return skui::Key::kVolDown;
        case SDLK_POWER:         return skui::Key::kPower;

        default:                 return skui::Key::kNONE;
    }
}

static skui::ModifierKey get_modifiers(Uint16 mod) {
    skui::ModifierKey result = skui::ModifierKey::kNone;
    if (mod & KMOD_SHIFT) {
        result |= skui::ModifierKey::kShift;
    }
    if (mod & KMOD_CTRL) {
        result |= skui::ModifierKey::kControl;
    }
    if (mod & KMOD_ALT) {
        result |= skui::ModifierKey::kOption;
    }
    if (mod & KMOD_GUI) {
        result |= skui::ModifierKey::kCommand;
    }
    return result;
}

void Window_sdl2::handleKeyEvent(const SDL_KeyboardEvent& event) {
    skui::Key key = get_key(event.keysym.sym);
    if (key == skui::Key::kNONE) {
        return;
    }
    skui::InputState state = (event.state == SDL_PRESSED) ? skui::InputState::kDown
                                                          : skui::InputState::kUp;
    skui::ModifierKey modifiers = get_modifiers(event.keysym.mod);
    if (!this->onKey(key, state, modifiers) &&
        key   == skui::Key::kEscape         &&
        state == skui::InputState::kDown) {
        this->closeWindow();
    }
}

void Window_sdl2::handleText(const char* begin) {
    // TODO: Do we need to ignore keys if they were already handled by handleKeyEvent?

    // The string we get from SDL is null terminated UTF-8 (there is no explicit length specified).
    const char* end = begin + strlen(begin);
    skui::ModifierKey modifiers = get_modifiers(SDL_GetModState());
    for (SkUnichar c = SkUTF::NextUTF8(&begin, end); c != -1; c = SkUTF::NextUTF8(&begin, end)) {
        this->onChar(c, modifiers);
    }
}

void Window_sdl2::handleMouseButtonEvent(const SDL_MouseButtonEvent& event) {
    if (event.button != SDL_BUTTON_LEFT) {
        // We only process the left button.
        return;
    }
    skui::InputState state = (event.state == SDL_PRESSED) ? skui::InputState::kDown
                                                          : skui::InputState::kUp;
    this->onMouse(event.x, event.y, state, get_modifiers(SDL_GetModState()));
}

void Window_sdl2::handleMouseMotionEvent(const SDL_MouseMotionEvent& event) {
    this->onMouse(event.x, event.y, skui::InputState::kMove, get_modifiers(SDL_GetModState()));
}

void Window_sdl2::handleMouseWheelEvent(const SDL_MouseWheelEvent& event) {
    this->onMouseWheel(event.y, get_modifiers(SDL_GetModState()));
}

void Window_sdl2::setTitle(const char* title) { SDL_SetWindowTitle(fWindow, title); }

void Window_sdl2::show() { SDL_ShowWindow(fWindow); }

bool Window_sdl2::attach(BackendType attachType) {
    if (fBackend == attachType && fWindowContext) {
        return true;
    }

    // In SDL we specify which 3D API we can create when fWindow is created. So if we're switching
    // we will respecify.
    if (fBackend != attachType) {
        InitWindowState initState;
        SkString titleStorage;
        if (fWindow) {
            titleStorage.printf("%s", SDL_GetWindowTitle(fWindow));
            initState.title = titleStorage.c_str();
            initState.hidden = SDL_GetWindowFlags(fWindow) & SDL_WINDOW_HIDDEN;
            // On Linux/GNOME the window seems to hop vertically when recreated at the "same"
            // position. Maybe related to this:
            // https://discourse.libsdl.org/t/sdl-get-setwindowposition-on-linux-have-different-origin/20073
            // But that'd be surprising given the age of that issue.
            SDL_GetWindowPosition(fWindow, &initState.pos.fX, &initState.pos.fY);
            SDL_GetWindowSize(fWindow, &initState.size.fWidth, &initState.size.fHeight);
        }
        this->closeWindow();
        fBackend = attachType;
        this->initWindow(initState);
    }

    window_context_factory::SDL2WindowInfo info;
    info.window = fWindow;

    switch (attachType) {
#ifdef SK_DAWN
        case kDawn_BackendType:
            fWindowContext =
                    window_context_factory::MakeDawnVulkanForSDL2(info, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_VULKAN
        case kVulkan_BackendType:
            fWindowContext =
                    window_context_factory::MakeVulkanForSDL2(info, fRequestedDisplayParams);
            break;
#endif
#ifdef SK_GL
        case kNativeGL_BackendType:
            fWindowContext = window_context_factory::MakeGLForSDL2(info, fRequestedDisplayParams);
            break;
#endif
        case kRaster_BackendType:
            fWindowContext =
                    window_context_factory::MakeRasterForSDL2(info, fRequestedDisplayParams);
            break;
    }
    this->onBackendCreated();

    return (SkToBool(fWindowContext));
}

void Window_sdl2::onInval() {
    SDL_Event event;
    event.type = SDL_WINDOWEVENT;
    event.window.event = SDL_WINDOWEVENT_EXPOSED;
    event.window.windowID = SDL_GetWindowID(fWindow);
    SDL_PushEvent(&event);
}

void Window_sdl2::setRequestedDisplayParams(const DisplayParams& params, bool allowReattach) {
    INHERITED::setRequestedDisplayParams(params, allowReattach);
}

const char* Window_sdl2::getClipboardText() { return fClipboardText.retrieve(); }

void Window_sdl2::setClipboardText(const char* text) { SDL_SetClipboardText(text); }

}  // namespace sk_app
