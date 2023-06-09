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
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkSamplingPriv.h"
#include "tools/ToolUtils.h"

extern int gOverrideMaxTextureSize;
extern std::atomic<int> gNumTilesDrawn;

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
    // We're using the knowledge that the internal tile size is 1024. By creating kImageSize
    // sized images we know we'll get a 4x4 tiling regardless of the sampling.
    static const int kImageSize = 4096 - 4 * 2 * kBicubicFilterTexelPad;
    static const int kOverrideMaxTextureSize = 1024;
    static const int kDesiredDepth = 8;
    static const int kDesiredLineWidth = 8;
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
        return SkISize::Make(kImageSize, kImageSize);
    }

    void onOnceBeforeDraw() override {
        SkBitmap bm;

        bm.allocN32Pixels(kImageSize, kImageSize, true);

        SkCanvas canvas(bm);

        int desiredDrawSize = kImageSize - 2*kPad;
        ToolUtils::HilbertGenerator gen(desiredDrawSize, kDesiredLineWidth, kDesiredDepth);

        canvas.translate(kPad, kImageSize - kPad);
        gen.draw(&canvas);

        fImg = bm.asImage();
    }

    void onDraw(SkCanvas* canvas) override {
        SkSamplingOptions sampling = SkSamplingOptions(SkCubicResampler::Mitchell());

        // Force 4x4 tiling
        gOverrideMaxTextureSize = kOverrideMaxTextureSize;

        canvas->drawImage(fImg, 0, 0, sampling);

        int actualNumTiles = gNumTilesDrawn.load(std::memory_order_acquire);
        SkASSERT(actualNumTiles == 16);
    }

private:
    sk_sp<SkImage> fImg;

};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
