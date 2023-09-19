/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkVertices.h"
#include "src/base/SkRandom.h"


namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularity of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("arcofzorro"); }

    SkISize getISize() override { return SkISize::Make(128, 128); }

    void onDraw(SkCanvas* canvas) override {

        SkPaint p;

        p.setShader(SkShaders::Color(SK_ColorRED));
        p.setColor({0.0f, 0.0f, 1.0f, 0.75f});

//        auto verts = make_vertices(SkRect::MakeWH(128, 128), SK_ColorGREEN);

//        canvas->drawVertices(verts, SkBlendMode::kPlus, p);

        canvas->drawRect(SkRect::MakeWH(128, 129), p);
    }

private:
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
