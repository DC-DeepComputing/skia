/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"

#include <cmath>

namespace skiagm {

class AndroidPoCGM : public GM {
public:
    AndroidPoCGM(bool hairline) : fHairline(hairline) {}

protected:
    SkString getName() const override {
        return fHairline ? SkString("android_poc_hairline") : SkString("android_poc");
    }

    SkISize getISize() override { return SkISize::Make(1400, 1800); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->clear(SkColorSetARGB(0xFF, 0x02, 0x0A, 0x08));

        // SVG viewBox="-100 -500 1400 1800"
        canvas->translate(100.f, 500.f);

        // SVG transform="matrix(4277 0 0 -4585 -69251 260221)"
        SkMatrix m = SkMatrix::MakeAll(4277.f,     0.f, -69251.f,
                                          0.f, -4585.f,  260221.f,
                                          0.f,     0.f,       1.f);
        canvas->concat(m);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SkColorSetARGB(0x29, 0x4A, 0xDE, 0x80)); // #4ade8029
        paint.setStrokeCap(SkPaint::kRound_Cap);
        paint.setStrokeJoin(SkPaint::kRound_Join);

        if (fHairline) {
            paint.setStrokeWidth(0.f); // Hairline stroke
        } else {
            paint.setStrokeWidth(1.1f);
        }

        const int pathCount = 30;
        const int subpathCount = 4800;
        int index = 0;

        for (int pathIndex = 0; pathIndex < pathCount; ++pathIndex) {
            SkPathBuilder builder;
            int end = (subpathCount * (pathIndex + 1)) / pathCount;
            for (; index < end; ++index) {
                float x = 16.205f + (float)(index % 80) / 320.0f;
                float y = 56.54f + (float)((index % 4800) / 80) / 250.0f;
                builder.moveTo(x, y);
                float curX = x;
                float curY = y;
                for (int segment = 0; segment < 6; ++segment) {
                    float dx = std::cos((float)(index + segment)) / 1000.0f;
                    float dy = std::sin((float)(index + segment)) / 1000.0f;
                    curX += dx;
                    curY += dy;
                    builder.lineTo(curX, curY);
                }
            }
            canvas->drawPath(builder.detach(), paint);
        }

        return DrawResult::kOk;
    }

private:
    bool fHairline;
};

DEF_GM(return new AndroidPoCGM(/*hairline=*/false);)
DEF_GM(return new AndroidPoCGM(/*hairline=*/true);)

}  // namespace skiagm
