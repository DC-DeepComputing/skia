/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(100, 100); }

    void onDraw(SkCanvas* canvas) override {
#if 0
        SkPath path;
        path.setFillType(SkPathFillType::kWinding);
        path.moveTo(12, 31);
        path.lineTo(12, 16);
        path.conicTo(12, 6, 22, 6, 0.707106769f);
        path.lineTo(234, 6);
        path.conicTo(244, 6, 244, 16, 0.707106769f);
        path.lineTo(244, 31);
        path.conicTo(244, 41, 234, 41, 0.707106769f);
        path.lineTo(22, 41);
        path.conicTo(12, 41, 12, 31, 0.707106769f);
        path.close();

        SkRect bounds = path.getBounds();
        SkRRect rrect;
        path.isRRect(&rrect);

        canvas->clipPath(path);

        canvas->drawRect({0, 0, 1024, 1024}, SkPaint());
#endif

        canvas->clear(SK_ColorGREEN);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
