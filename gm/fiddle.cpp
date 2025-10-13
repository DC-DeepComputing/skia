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
    canvas->clear(SK_ColorRED);

    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeXYWH(20.f, 20.f, 200.f, 200.f), 49.999f, 50.001f);


    SkPaint greenFill;
    greenFill.setColor(SK_ColorGREEN);
    greenFill.setAntiAlias(true);
    greenFill.setStroke(false);

    SkPaint blueStroke;
    blueStroke.setColor(SK_ColorBLUE);
    blueStroke.setAntiAlias(true);
    blueStroke.setStroke(true);
    blueStroke.setStrokeWidth(1.f);

    auto drawDRRect = [&](SkCanvas* canvas) {
        SkAutoCanvasRestore acr{canvas, /*doSave=*/true};
        SkRRect outer = rrect;
        outer.outset(blueStroke.getStrokeWidth(), blueStroke.getStrokeWidth());

        SkPaint blueFill = blueStroke;
        blueFill.setStroke(false);
        canvas->drawDRRect(outer, rrect, blueFill);
        canvas->drawRRect(rrect, greenFill);
    };

    canvas->translate(rrect.rect().right() + 20.f, 0.f);
    auto drawRRect = [&](SkCanvas* canvas) {
        SkAutoCanvasRestore acr{canvas, /*doSave=*/true};
        SkRRect stroke = rrect;
        stroke.outset(0.5f * blueStroke.getStrokeWidth(),
                      0.5f * blueStroke.getStrokeWidth());
        canvas->drawRRect(stroke, blueStroke);
        canvas->drawRRect(rrect, greenFill);
    };

    drawDRRect(canvas);
    canvas->translate(rrect.rect().right() + 20.f, 0.f);
    drawRRect(canvas);

    SkImageInfo ii = canvas->imageInfo().makeWH(240, 240);
    sk_sp<SkSurface> drrectS = canvas->makeSurface(ii);
    drawDRRect(drrectS->getCanvas());
    sk_sp<SkImage> drrectI = drrectS->makeTemporaryImage();

    sk_sp<SkSurface> rrectS = canvas->makeSurface(ii);
    drawRRect(rrectS->getCanvas());
    sk_sp<SkImage> rrectI = rrectS->makeTemporaryImage();

    SkPaint diffPaint;
    SkRuntimeEffect::ChildPtr children[] = {drrectI->makeShader(SkFilterMode::kNearest),
                                            rrectI->makeShader(SkFilterMode::kNearest)};
    diffPaint.setShader(SkRuntimeEffect::MakeForShader(SkString(
        "uniform shader s1;"
        "uniform shader s2;"
        "half4 main(float2 coord) {"
            "half4 diff = abs(s1.eval(coord) - s2.eval(coord));"
            "half m = max(max(diff.x, diff.y), max(diff.z, diff.w));"
            "return half4(half3(m * 16), 1.0);"
        "}"
    )).effect->makeShader(nullptr, children));

    canvas->translate(-rrect.rect().right() - 20.f,
                       rrect.rect().height() + 40.f);
    canvas->drawRect(SkRect::Make(ii.bounds()), diffPaint);

    canvas->translate(0.f, rrect.height() + 40.f);

    {
        SkRect bounds = SkRect::MakeLTRB(0.f, 0.f, 854.f, 480.f);
        SkRRect clip = SkRRect::MakeRectXY(SkRect::MakeLTRB(0.f, -36.375618f, 854.f, 823.986816f), 48.5464821f, 48.548542f);

        SkPaint solidPaint;
        solidPaint.setAntiAlias(false);
        solidPaint.setStroke(false);
        solidPaint.setColor(SK_ColorBLACK);

        SkPaint clipPaint;
        clipPaint.setAntiAlias(true);
        clipPaint.setStroke(true);
        clipPaint.setColor(SK_ColorRED);

        SkPaint solidBorderPaint;
        solidBorderPaint.setAntiAlias(false);
        solidBorderPaint.setStroke(true);
        solidBorderPaint.setColor(SK_ColorGREEN);

        canvas->save();
        canvas->clipRRect(clip, true);
        canvas->drawRect(bounds, solidPaint);

        canvas->restore();

        canvas->drawRect(bounds, solidBorderPaint);
        canvas->drawRRect(clip, clipPaint);
    }

    canvas->translate(rrect.width() + 40.f, -rrect.height() - 40.f);

    // {
    //     SkAutoCanvasRestore acr{canvas, true};
    //     sk_sp<SkMaskFilter> blur = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 4.f);

    //     SkVector cornerRadii[4] = {{0.f, 0.f},
    //                                {15.f, 40.f},
    //                                {30.f, 10.f},
    //                                {60., 60.f}};

    //     SkRRect complexRRect;
    //     complexRRect.setRectRadii(SkRect::MakeLTRB(0.f, 0.f, 150.f, 200.f), cornerRadii);

    //     SkPaint p;
    //     p.setColor(SK_ColorBLUE);
    //     p.setMaskFilter(blur);

    //     SkPaint s;
    //     s.setColor(SK_ColorBLACK);
    //     s.setStroke(true);
    //     s.setAntiAlias(true);

    //     canvas->drawRRect(complexRRect, p);
    //     canvas->drawRRect(complexRRect, s);

    //     canvas->translate(complexRRect.width() + 50.f, 0.f);

    //     canvas->scale(2.f, 1.f);
    //     canvas->drawRRect(complexRRect, p);
    //     canvas->drawRRect(complexRRect, s);

    //     canvas->scale(0.5f, 1.f);
    //     canvas->translate(2*complexRRect.width() + complexRRect.height() + 50.f, 0.f);
    //     canvas->rotate(90.f);
    //     canvas->scale(2.f, 1.f);
    //     canvas->drawRRect(complexRRect, p);
    //     canvas->drawRRect(complexRRect, s);
    // }
}
