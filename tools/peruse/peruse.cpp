/**
 *  Copyright 2015 Mike Reed
 */

#include "tools/peruse/window.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

class TestWindow : public GWindow {
public:
    TestWindow(SkISize sz) : GWindow(sz) {
    }

protected:
    void onDraw(SkCanvas* canvas) override {
        canvas->clear({1, 0, 0, 1});

        SkPaint paint({0, 0, 1, 1});
        canvas->drawRect({100, 50, 300, 200}, paint);
    }

    bool onKeyPress(uint32_t sym) override {
        return false;
    }

    std::unique_ptr<GClick> onFindClickHandler(SkPoint loc) override {
        return nullptr;
    }

private:
    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow({600, 600});

    return wind->run();
}
