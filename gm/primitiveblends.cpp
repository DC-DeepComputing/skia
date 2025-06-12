/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"

// #include "include/core/SkColor.h"
// #include "include/core/SkFont.h"
// #include "include/core/SkImageInfo.h"
// #include "include/core/SkPaint.h"
// #include "include/core/SkPoint.h"
// #include "include/core/SkRect.h"
// #include "include/core/SkRefCnt.h"
// #include "include/core/SkScalar.h"
// #include "include/core/SkShader.h"
// #include "include/core/SkSize.h"
// #include "include/core/SkString.h"
// #include "include/core/SkSurface.h"
// #include "include/core/SkTileMode.h"
// #include "include/core/SkTypeface.h"
// #include "include/core/SkTypes.h"
// #include "include/effects/SkGradientShader.h"
// #include "tools/ToolUtils.h"
// #include "tools/fonts/FontToolUtils.h"

namespace skiagm {

constexpr int kColWidth = 180;
constexpr int kNumCols = 4;
constexpr int kWidth = kColWidth * kNumCols;
constexpr int kHeight = 750;

static sk_sp<SkShader> make_shader(const SkRect& bounds) {
    const SkPoint pts[] = {
        { bounds.left(), bounds.top() },
        { bounds.right(), bounds.bottom() },
    };
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN,
    };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                        SkTileMode::kRepeat);
}

class PrimitiveBlendGM : public skiagm::GM {
public:
    PrimitiveBlendGM() {
        // const int kPointSize = 25;
        // fTextHeight = SkIntToScalar(kPointSize);
    }

protected:
    SkString getName() const override { return SkString("primitiveblends"); }

    void onOnceBeforeDraw() override {
        // fCheckerboard = ToolUtils::create_checkerboard_shader(SK_ColorBLACK, SK_ColorWHITE, 4);
    }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(false);
        p.setStyle(SkPaint::kFill_Style);
        p.setShader(fCheckerboard);
        SkRect r = SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        canvas->drawRect(r, p);

        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
        SkSurfaceProps props = SkSurfaceProps(0, kRGB_H_SkPixelGeometry);
        auto surface(ToolUtils::makeSurface(canvas, info, &props));

        SkCanvas* surfCanvas = surface->getCanvas();

        SkPaint surfPaint;
        surfPaint.setBlendMode(SkBlendMode::kSrcOver);
        surface->draw(canvas, 0, 0, SkSamplingOptions(), &surfPaint);

    //     const SkBlendMode gModes[] = {
    //         SkBlendMode::kClear,
    //         SkBlendMode::kSrc,
    //         SkBlendMode::kDst,
    //         SkBlendMode::kSrcOver,
    //         SkBlendMode::kDstOver,
    //         SkBlendMode::kSrcIn,
    //         SkBlendMode::kDstIn,
    //         SkBlendMode::kSrcOut,
    //         SkBlendMode::kDstOut,
    //         SkBlendMode::kSrcATop,
    //         SkBlendMode::kDstATop,
    //         SkBlendMode::kXor,
    //         SkBlendMode::kPlus,
    //         SkBlendMode::kModulate,
    //         SkBlendMode::kScreen,
    //         SkBlendMode::kOverlay,
    //         SkBlendMode::kDarken,
    //         SkBlendMode::kLighten,
    //         SkBlendMode::kColorDodge,
    //         SkBlendMode::kColorBurn,
    //         SkBlendMode::kHardLight,
    //         SkBlendMode::kSoftLight,
    //         SkBlendMode::kDifference,
    //         SkBlendMode::kExclusion,
    //         SkBlendMode::kMultiply,
    //         SkBlendMode::kHue,
    //         SkBlendMode::kSaturation,
    //         SkBlendMode::kColor,
    //         SkBlendMode::kLuminosity,
    //     };
    }

private:
    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PrimitiveBlendGM; )
}  // namespace skiagm
