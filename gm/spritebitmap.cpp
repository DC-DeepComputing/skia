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
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"

#include <utility>

static void make_bm(SkBitmap* bm) {
    bm->allocN32Pixels(256, 256);
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            SkIRect irect = SkIRect::MakeXYWH(x, y, 1, 1);
            SkColor color = SkColorSetRGB(x, y , 0);
            bm->eraseArea(irect, color);
        }
    }
}

class MagicColorSpaceGM : public skiagm::GM {
public:
    MagicColorSpaceGM() {}

protected:

    SkString onShortName() override {
        return SkString("colorspace_sin");
    }

    SkISize onISize() override {
        constexpr const int size = 256 + Border*2;
        return SkISize::Make(size, size);
    }

    void onDraw(SkCanvas* canvas) override {
        SkBitmap bm;
        make_bm(&bm);

        canvas->drawImage(bm.asImage(), SkIntToScalar(Border), SkIntToScalar(Border),
                          SkSamplingOptions());
    }

private:
    static constexpr const int Border = 10;
    using INHERITED = GM;
};
DEF_GM( return new MagicColorSpaceGM; )
