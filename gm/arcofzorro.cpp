/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
    static const int kSize = 1024;
    static const int kDesiredDepth = 5;
    static const int kDesiredLineWidth = 16;
    static const int kPad = kDesiredLineWidth;

public:
    ArcOfZorroGM() {
        this->setBGColor(SK_ColorDKGRAY);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(kSize, kSize);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bm;

        bm.allocN32Pixels(kSize, kSize, true);

        SkCanvas canvas(bm);

        int desiredDrawSize = kSize - 2*kPad;
        ToolUtils::HilbertGenerator gen(desiredDrawSize, kDesiredLineWidth, kDesiredDepth);

        canvas.translate(kPad, kSize - kPad);
        gen.draw(&canvas);

        fImg = bm.asImage();
    }

    void onDraw(SkCanvas* canvas) override {
        SkSamplingOptions sampling = SkSamplingOptions(SkCubicResampler::Mitchell());

        canvas->drawImage(fImg, 0, 0, sampling);
    }

private:
    sk_sp<SkImage> fImg;

};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
