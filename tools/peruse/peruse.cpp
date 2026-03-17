/**
 *  Copyright 2015 Mike Reed
 */

#include "tools/peruse/window.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"

#include "gm/gm.h"

struct GMRec {

    skiagm::GMFactory   fFactory;
    SkString            fName;

    skiagm::DrawResult draw(SkCanvas* canvas) {
        auto gm = fFactory();

        SkString msg;

        skiagm::DrawResult gpuSetupResult = gm->gpuSetup(canvas, &msg, nullptr);
        switch (gpuSetupResult) {
            case skiagm::DrawResult::kOk  : break;
            case skiagm::DrawResult::kFail: [[fallthrough]];
            case skiagm::DrawResult::kSkip: return gpuSetupResult;
            default: SK_ABORT("");
        }

        return gm->draw(canvas, &msg);
    }
};

static std::vector<GMRec> gGMs;

class TestWindow : public GWindow {
    size_t  fGMIndex = 0;

public:
    TestWindow(SkISize sz) : GWindow(sz) {
    }

protected:
    void onDraw(SkCanvas* canvas) override {
        canvas->clear({1, 1, 1, 1});
        gGMs[fGMIndex].draw(canvas);
    }

    bool onKeyPress(uint32_t sym) override {
        switch (sym) {
            case SDLK_LEFT:
                fGMIndex = (fGMIndex - 1) % gGMs.size();
                this->requestDraw();
                return true;
            case SDLK_RIGHT:
                fGMIndex = (fGMIndex + 1) % gGMs.size();
                this->requestDraw();
                return true;
            default:
                break;
        }
        return false;
    }

    std::unique_ptr<GClick> onFindClickHandler(SkPoint loc) override {
        return nullptr;
    }

private:
    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    for (const skiagm::GMFactory& f : skiagm::GMRegistry::Range()) {
        auto gm = f();
        gGMs.push_back({f, gm->getName()});
        printf("%s\n", gm->getName().c_str());
    }

    GWindow* wind = new TestWindow({600, 600});

    return wind->run();
}
