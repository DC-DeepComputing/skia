/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GWindow_DEFINED
#define GWindow_DEFINED

#include <SDL2/SDL.h>
#include <functional>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "include/core/SkBitmap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

class SkCanvas;
class GClick;

static inline SkPoint operator*(const SkMatrix& m, SkPoint p) {
    return m.mapPoint(p);
}

enum GFastKeys {
    left_arrow =  1 << 0,
    right_arrow = 1 << 1,
    up_arrow =    1 << 2,
    down_arrow =  1 << 3,
    return_key =  1 << 4,
    space_key =   1 << 5,

    ctrl_key  = 1 << 6,
    opt_key   = 1 << 7,
};

class GClick {
public:
    GClick(SkPoint, std::function<void(const GClick*)>);

    enum class State {
        kDown,
        kMove,
        kUp
    };

    State state() const { return fState; }
    bool isUp() const { return fState == State::kUp; }

    SkPoint curr() const { return fCurr; }
    SkPoint prev() const { return fPrev; }
    SkPoint orig() const { return fOrig; }

    void callback() { fFunc(this); }

    static std::unique_ptr<GClick> Noop();

private:
    SkPoint  fCurr, fPrev, fOrig;
    State   fState;
    std::function<void(const GClick*)> fFunc;

    friend class GWindow;
};

class GWindow {
public:
    int run();

    void requestDraw();

protected:
    GWindow(SkISize);
    virtual ~GWindow();

    SkISize size() const { return fSize; }
    SkIRect ibounds() const { return {0, 0, fSize.width(), fSize.height()}; }
    SkRect bounds() const { return SkRect::Make(this->ibounds()); }

    uint32_t pollFastKeys() const;

    virtual void onUpdate(SkCanvas*);
    virtual void onDraw(SkCanvas*) {}
    virtual void onResize(SkISize) {}
    virtual bool onKeyPress(uint32_t) { return false; }
    virtual void onKeyUp(uint32_t) {}
    virtual void onFastKeys(uint32_t mask) {}
    virtual std::unique_ptr<GClick> onFindClickHandler(SkPoint) { return nullptr; }

    void setTitle(const char title[]);

    SkMatrix ctm() const { return fCTM; }
    void setCtm(const SkMatrix& ctm) {
        if (fCTM != ctm) {
            this->requestDraw();
        }
        fCTM = ctm;
    }

    static void main_loop(GWindow*);

private:
    std::unique_ptr<GClick> fClick;

    SkISize fSize;
    bool fNeedDraw;
    bool fQuit = false;

    SDL_Window*   fWindow;
    SDL_Renderer* fRenderer;
    SDL_Texture*  fTexture;
    const uint8_t* fKeyboardState;

    uint32_t fInvalEventType;

    SkMatrix fCTM;

    bool handleEvent(const SDL_Event&);
    void pushEvent(int code) const;
    void drawIntoTexture();
    void handleResize(int w, int h);
};

#endif
