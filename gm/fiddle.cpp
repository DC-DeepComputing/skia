/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPoint src[4];
    SkRect::MakeWH(120.f, 120.f).toQuad(src);
    SkPoint dst[4] = {{0, 0},
                      {120.f + 10.f, 15.f},
                      {120.f - 28.f, 120.f + 40.f},
                      {25.f, 120.f - 15.f}};
    SkMatrix m;
    SkAssertResult(m.setPolyToPoly(src, dst, 4));
    m.preTranslate(0.f, 10.f);

    SkPath p = SkPath::Rect(SkRect::MakeWH(120.f, 120.f));
    p.addCircle(60.f, 60.f, 20.f, SkPathDirection::kCCW);

    SkPaint paint;
    paint.setAntiAlias(true);
    static constexpr SkPoint kPts[] = { {0.f, 0.f}, {30.f, 30.f} };
    static constexpr SkColor kColors[] = { SK_ColorBLUE, SK_ColorWHITE };
    paint.setShader(SkGradientShader::MakeLinear(kPts, kColors, nullptr, 2, SkTileMode::kMirror));

    {
        canvas->save();
        canvas->concat(m);
        // With a path, this will trigger MF->IF flow, but there's perspective in the CTM so it'll
        // decompose into a non-perspective pre-filter transform, and a mask-to-device matrix that
        // has perspective on it.
        paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 10.f));
        canvas->drawPath(p, paint);
        canvas->restore();
    }

    canvas->translate(120.f, 0.f);

    {
        canvas->save();
        canvas->concat(m);
        // With a shader mask filter, which supports complex transforms, this will not decompose
        // the transform, so the mask-to-device matrix will be translation only and the local matrix
        // will have to reconstruct the perspective local coordinate interpolation.
        static constexpr SkColor kAlphas[] = { SK_ColorTRANSPARENT, SK_ColorBLACK };
        auto alphaGradient = SkGradientShader::MakeRadial(
                {60.f, 60.f}, 40.f, kAlphas, nullptr, 2, SkTileMode::kClamp);

        paint.setMaskFilter(SkShaderMaskFilter::Make(std::move(alphaGradient)));
        canvas->drawPath(p, paint);
        canvas->restore();
    }

    canvas->translate(120.f, 0.f);

    {
        canvas->save();
        canvas->concat(m);
        // No mask filter for reference
        paint.setMaskFilter(nullptr);
        canvas->drawPath(p, paint);
        canvas->restore();
    }
}
