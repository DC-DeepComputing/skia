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

#include "include/effects/SkGradientShader.h"

namespace {

class HilbertGenerator {
public:
    HilbertGenerator(float desiredSize, int desiredDepth, int width)
            : fDesiredDepth(desiredDepth)
            , fWidth(width)
            , fActualBounds(SkRect::MakeEmpty())
            , fCurPos(SkPoint::Make(0.0f, 0.0f))
            , fCurDir(0) {
        SkDebugf("desiredSize %f\n", desiredSize);
        fLength = desiredSize / ((0x1 << fDesiredDepth) - 1.0f);
        fExpectedLen = fLength * ((0x1 << (2*fDesiredDepth)) - 1.0f);
    }

    void draw(SkCanvas* canvas) {
        float curLen = 0.0f;
        this->recursiveDraw(canvas, /* curDepth= */ 0, /* turnRight= */ true, &curLen);
        SkScalarNearlyEqual(curLen, fExpectedLen, 0.01f);

        SkDebugf("bounds: %.2f %.2f %.2f %.2f width %f height %f\n",
                 fActualBounds.fLeft, fActualBounds.fTop, fActualBounds.fRight, fActualBounds.fBottom,
                 fActualBounds.width(), fActualBounds.height());

        SkDebugf("len %f actualLen %f expectedLen %f -- actual/len = %f -- expected/len = %f\n",
                 fLength, curLen, fExpectedLen, curLen/fLength, fExpectedLen/fLength);
    }

private:
    void turn90(bool turnRight) {
        fCurDir += turnRight ? 90 : -90;
        if (fCurDir >= 360) {
            fCurDir = 0;
        } else if (fCurDir < 0) {
            fCurDir = 270;
        }

        SkASSERT(fCurDir == 0 || fCurDir == 90 || fCurDir == 180 || fCurDir == 270);
    }

#if 0
        void curve(SkCanvas* canvas) {
            SkRect r;
            SkPaint paint;
            paint.setColor(SK_ColorRED);

            canvas->drawArc(r, 0, 360, /* useCenter= */ false, paint);
        }
#endif

    void line(SkCanvas* canvas, float* actualLen) {

        SkPoint before = fCurPos;

        SkRect r;
        switch (fCurDir) {
            case 0:
                r.fLeft = fCurPos.fX;
                r.fTop = fCurPos.fY - fWidth / 2.0f;
                r.fRight = fCurPos.fX + fLength;
                r.fBottom = fCurPos.fY + fWidth / 2.0f;
                fCurPos.fX += fLength;
                break;
            case 90:
                r.fLeft = fCurPos.fX - fWidth / 2.0f;
                r.fTop = fCurPos.fY - fLength;
                r.fRight = fCurPos.fX + fWidth / 2.0f;
                r.fBottom = fCurPos.fY;
                fCurPos.fY -= fLength;
                break;
            case 180:
                r.fLeft = fCurPos.fX - fLength;
                r.fTop = fCurPos.fY - fWidth / 2.0f;
                r.fRight = fCurPos.fX;
                r.fBottom = fCurPos.fY + fWidth / 2.0f;
                fCurPos.fX -= fLength;
                break;
            case 270:
                r.fLeft = fCurPos.fX - fWidth / 2.0f;
                r.fTop = fCurPos.fY;
                r.fRight = fCurPos.fX + fWidth / 2.0f;
                r.fBottom = fCurPos.fY + fLength;
                fCurPos.fY += fLength;
                break;
            default:
                return;
        }

        SkPoint pts[2] = { before, fCurPos };

        SkColor4f colors[2] = {
            this->getColor(*actualLen),
            this->getColor(*actualLen + fLength),
        };

        *actualLen += fLength;
        if (fActualBounds.isEmpty()) {
            fActualBounds = r;
        } else {
            fActualBounds.join(r);
        }

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setShader(SkGradientShader::MakeLinear(pts, colors, /* colorSpace= */ nullptr,
                                                     /* pos= */ nullptr, 2, SkTileMode::kClamp));
        canvas->drawRect(r, paint);

//        SkDebugf("drawing %.2f, %.2f to %.2f, %.2f - angle: %d length: %f\n", before.fX,
//                 before.fY,
//                 fCurPos.fX, fCurPos.fY, fCurDir, fLength);
    }

    void recursiveDraw(SkCanvas* canvas,
                       int curDepth,
                       bool turnRight,
                       float* curLen) {
        if (curDepth >= fDesiredDepth) {
            return;
        }

        this->turn90(turnRight);
        this->recursiveDraw(canvas, curDepth + 1, !turnRight, curLen);
        this->line(canvas, curLen);
        this->turn90(!turnRight);
        this->recursiveDraw(canvas, curDepth + 1, turnRight, curLen);
        this->line(canvas, curLen);
        this->recursiveDraw(canvas, curDepth + 1, turnRight, curLen);
        this->turn90(!turnRight);
        this->line(canvas, curLen);
        this->recursiveDraw(canvas, curDepth + 1, !turnRight, curLen);
        this->turn90(turnRight);
    }

    SkColor4f getColor(float curLen) {
        static const SkColor4f kColors[] = {
                SkColors::kBlack,
                SkColors::kBlue,
                SkColors::kCyan,
                SkColors::kGreen,
                SkColors::kYellow,
                SkColors::kRed,
                SkColors::kWhite,
        };

        static const float kStops[] = {
                0.0f,
                1.0f/6.0f,
                2.0f/6.0f,
                0.5f,
                4.0f/6.0f,
                5.0f/6.0f,
                1.0f,
        };

        float t = curLen / fExpectedLen;
        if (t <= 0.0f) {
            return kColors[0];
        } else if (t >= 1.0f) {
            return kColors[6];
        }

        for (int i = 0; i < 6; ++i) {
            if (kStops[i] <= t && t <= kStops[i+1]) {
                t -= kStops[i];
                t *= 6;
                SkASSERT(0.0f <= t && t <= 1.0f);
                return { kColors[i].fR * (1 - t) + kColors[i+1].fR * t,
                         kColors[i].fG * (1 - t) + kColors[i+1].fG * t,
                         kColors[i].fB * (1 - t) + kColors[i+1].fB * t,
                         kColors[i].fA * (1 - t) + kColors[i+1].fA * t };

            }
        }

        return SkColors::kBlack;
    }

    float fLength;
    const int fDesiredDepth;
    const int fWidth;
    SkRect fActualBounds;
    SkPoint fCurPos;
    int fCurDir;
    float fExpectedLen;
};

} // anonymous namespace

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
    static const int kSize = 256;
    static const int kLineWidth = 6;
    static const int kDesiredDepth = 5;
    static const int kPad = kLineWidth;

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

        HilbertGenerator gen(/* desiredSize= */ kSize - 2*kPad,
                             kDesiredDepth,
                             kLineWidth);

        canvas.translate(kPad, kSize - kPad);
        gen.draw(&canvas);

        fImg = bm.asImage();
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawImage(fImg, 0, 0);
    }

private:
    sk_sp<SkImage> fImg;

};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}  // namespace skiagm
