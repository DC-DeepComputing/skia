/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrRecordingContext.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {

static const int kWidth = 256;
static const int kHeight = 256;

SkPoint create_center(SkRandom* rand) {
    SkPoint c = { rand->nextRangeF(0.0f, kWidth), rand->nextRangeF(0.0f, kHeight) };
    return c;
}

SkPath create_concave_path(SkRandom* rand, const SkPoint& c) {
    SkPath p;
    int numLines = rand->nextRangeU(4, 8);
    float angleStep = 2.0 * SK_FloatPI / numLines;

    float length = rand->nextRangeF(4.0f, 32);

    p.moveTo(c.fX-1.0f, c.fY);

    float angle = angleStep;
    for (int i = 1; i < numLines; ++i) {
        length += rand->nextRangeF(-4.0f, 4.0f);
        float x = length * SkScalarCos(angle);
        float y = length * SkScalarSin(angle);

        p.lineTo(c.fX + x, c.fY + y);

        angle += angleStep;
    }
    p.close();

//    SkASSERT(!p.isConvex());
    return p;
}

} // anonymous namespace

namespace skiagm {

// This is just for bootstrapping Graphite.
class GraphiteStartGM : public GM {
public:
    GraphiteStartGM() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:


    SkString onShortName() override {
        return SkString("graphitestart");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        for (int i = 0; i < 5; ++i) {
            SkPoint c = create_center(&fRand);

            SkPath p1 = create_concave_path(&fRand, c);
            SkRect r1 = p1.getBounds();
            r1.makeOutset(0.1f*r1.width(), 0.1f*r1.height());

            SkPath p2 = create_concave_path(&fRand, c);
            SkRect r2 = p2.getBounds();
            r2.makeOutset(0.1f*r2.width(), 0.1f*r2.height());

            r1.join(r2);

            SkPaint paint;
            paint.setColor(ToolUtils::color_to_565(fRand.nextU() | (0xFF << 24)));
            paint.setAntiAlias(true);

            canvas->save();
//            canvas->clipPath(p1);
//            canvas->clipPath(p2, SkClipOp::kIntersect, true);
//            canvas->drawRect(r1, paint);
            canvas->drawPath(p1, paint);
            canvas->restore();
        }

    }

private:
    SkRandom fRand;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new GraphiteStartGM;)

}  // namespace skiagm
