/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "tools/ToolUtils.h"

#include <array>

namespace skiagm {

class ImageFiltersSUnpremul : public GM {
public:
    ImageFiltersSUnpremul() {
        this->setBGColor(0x00000000);
    }

protected:
    constexpr static int square_side = 128;

    SkString onShortName() override {
        // Test draws an un-premultiplied image and verifies alpha channel was blended correctly.
        return SkString("imagefiltersunpremul");
    }

    SkISize onISize() override {
        return SkISize::Make(square_side, square_side);
    }

    void onOnceBeforeDraw() override {
        std::array<uint32_t, square_side * square_side> pixelArray;
        SkPixmap pixmap(
                SkImageInfo::Make(square_side, square_side, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kUnpremul_SkAlphaType),
                pixelArray.data(),
                4 * square_side);
        SkBitmap bitmap;
        bitmap.installPixels(pixmap);
        SkCanvas canvas2(bitmap);
        canvas2.clear(SkColorSetARGB(50, 255, 0, 0));
        fUnpremulImage = SkImage::MakeRasterCopy(pixmap);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        canvas->clear(SK_ColorBLACK);
        auto filter = SkImageFilters::Image(fUnpremulImage);
        p.setImageFilter(filter);
        canvas->drawPaint(p);
    }

private:
    sk_sp<SkImage> fUnpremulImage;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageFiltersSUnpremul;)
}  // namespace skiagm
