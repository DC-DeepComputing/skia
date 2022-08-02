/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"

class DRRectGM : public skiagm::GM {
public:
    DRRectGM() {}

protected:

    SkString onShortName() override {
        return SkString("drrect");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        SkRRect outers[4];
        // like squares/circles, to exercise fast-cases in GPU
        SkRect r = { 0, 0, 100, 100 };
        SkVector radii[4] = {
            { 0, 0 }, { 30, 1 }, { 10, 40 }, { 40, 40 }
        };

        const SkScalar dx = r.width() + 16;
        const SkScalar dy = r.height() + 16;

        outers[0].setRect(r);
        outers[1].setOval(r);
        outers[2].setRectXY(r, 20, 20);
        outers[3].setRectRadii(r, radii);

        SkRRect inners[5];
        r.inset(25, 25);

        inners[0].setEmpty();
        inners[1].setRect(r);
        inners[2].setOval(r);
        inners[3].setRectXY(r, 20, 20);
        inners[4].setRectRadii(r, radii);

        canvas->translate(16, 16);
        for (size_t j = 0; j < std::size(inners); ++j) {
            for (size_t i = 0; i < std::size(outers); ++i) {
                canvas->save();
                canvas->translate(dx * j, dy * i);
                canvas->drawDRRect(outers[i], inners[j], paint);
                canvas->restore();
            }
        }
    }

private:
    using INHERITED = GM;
};

DEF_GM( return new DRRectGM; )

// From https://github.com/flutter/flutter/issues/108713
DEF_SIMPLE_GM(drrect_flutter_108713, canvas, 700, 400) {
    // yellow painter
    {
        SkRect innerRect = {317.8f, 181.f, 669.5f, 332.7f};
        SkVector innerRadii[4] = {
            {51.7f, 100.f},
            {100.f, 100.f},
            {100.f, 51.7f},
            {51.7f, 51.7f}
        };
        SkRect outerRect = {261.f, 181.f, 669.5f, 389.5f};
        SkVector outerRadii[4] = {
            {108.5f, 100.f},
            {100.f, 100.f},
            {100.f, 108.5f},
            {108.5f, 108.5f}
        };

        SkRRect inner;
        inner.setRectRadii(innerRect, innerRadii);
        SkRRect outer;
        outer.setRectRadii(outerRect, outerRadii);

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0x80ff8050);
        canvas->drawDRRect(outer, inner, p);
    }
    // green painter
    {
        SkRect innerRect = {253.4f, 132.f, 600.f, 278.6f};
        SkVector innerRadii[4] = {
            {3.9f, 57.3f},
            {57.3f, 57.3f},
            {57.3f, 3.9f},
            {3.9f, 3.9f}
        };
        SkRect outerRect = {195.2f, 132.f, 600.f, 336.8f};
        SkVector outerRadii[4] = {
            {62.2f, 57.3f},
            {57.3f, 57.3f},
            {57.3f, 62.2f},
            {62.2f, 62.2f}
        };

        SkRRect inner;
        inner.setRectRadii(innerRect, innerRadii);
        SkRRect outer;
        outer.setRectRadii(outerRect, outerRadii);

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0x80008050);
        canvas->drawDRRect(outer, inner, p);
    }
}
