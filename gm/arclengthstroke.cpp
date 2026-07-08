/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "src/core/SkPathEffectBase.h"

namespace skiagm {

class ArcLengthDummyPathEffect : public SkPathEffectBase {
public:
    static sk_sp<SkPathEffect> Make() {
        return sk_sp<SkPathEffect>(new ArcLengthDummyPathEffect());
    }

    bool isArcLengthParameterized() const override { return true; }

    bool computeFastBounds(SkRect* bounds) const override { return true; }

protected:
    bool onFilterPath(SkPathBuilder*, const SkPath&, SkStrokeRec*, const SkRect*,
                      const SkMatrix&) const override {
        return false;
    }

    Factory getFactory() const override { return nullptr; }
    const char* getTypeName() const override { return "ArcLengthDummyPathEffect"; }
    Type getFlattenableType() const override { return kSkPathEffect_Type; }

private:
    ArcLengthDummyPathEffect() = default;
};

class ArcLengthStrokeGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthstroke"); }

    SkISize getISize() override { return SkISize::Make(500, 500); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(30.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        // 1. A straight line
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 50.f)
                .lineTo(450.f, 50.f)
                .detach();
            canvas->drawPath(path, paint);
        }

        // 2. A quadratic curve
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 150.f)
                .quadTo(250.f, 300.f, 450.f, 150.f)
                .detach();
            canvas->drawPath(path, paint);
        }

        // 3. A cubic curve
        {
            SkPath path = SkPathBuilder()
                .moveTo(50.f, 350.f)
                .cubicTo(150.f, 200.f, 350.f, 500.f, 450.f, 350.f)
                .detach();
            canvas->drawPath(path, paint);
        }
    }
};

DEF_GM(return new ArcLengthStrokeGM;)

class ArcLengthSkiaGM : public GM {
protected:
    SkString getName() const override { return SkString("arclengthskia"); }

    SkISize getISize() override { return SkISize::Make(500, 220); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorBLACK);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(20.f);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);
        paint.setPathEffect(ArcLengthDummyPathEffect::Make());
        paint.setColor(SK_ColorWHITE);

        SkPath path = SkPathBuilder()
            // S
            .moveTo(140.f, 60.f)
            .cubicTo(110.f, 40.f, 60.f, 60.f, 80.f, 95.f)
            .cubicTo(95.f, 120.f, 135.f, 110.f, 135.f, 135.f)
            .cubicTo(135.f, 155.f, 90.f, 165.f, 60.f, 145.f)

            // k
            .moveTo(180.f, 40.f)
            .lineTo(180.f, 160.f)
            .moveTo(240.f, 70.f)
            .lineTo(180.f, 110.f)
            .lineTo(240.f, 160.f)

            // i
            .moveTo(280.f, 85.f)
            .lineTo(280.f, 160.f)
            .moveTo(280.f, 55.f)
            .lineTo(280.f, 56.f)

            // a
            .moveTo(380.f, 110.f)
            .cubicTo(380.f, 80.f, 320.f, 80.f, 320.f, 120.f)
            .cubicTo(320.f, 160.f, 380.f, 160.f, 380.f, 120.f)
            .lineTo(380.f, 160.f)
            .detach();

        canvas->drawPath(path, paint);
    }
};

DEF_GM(return new ArcLengthSkiaGM;)

}  // namespace skiagm
