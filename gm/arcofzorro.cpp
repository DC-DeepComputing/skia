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
            , fCurDir(0) {
        fLength = desiredSize / ((0x1 << fDesiredDepth) - 1.0f);
        fExpectedLen = fLength * ((0x1 << (fDesiredDepth+4)) - 1.0f);
    }

    void draw(SkCanvas* canvas) {
        SkPoint curPos{0.0f, 0.0f};

        float actualLen = 0.0f;
        this->recursiveDraw(canvas, /* curDepth= */ 0, &curPos, /* turnAmount= */ 90, &actualLen);
        SkScalarNearlyEqual(actualLen, fExpectedLen, 0.01f);

        SkDebugf("bounds: %.2f %.2f %.2f %.2f\n",
                 fActualBounds.fLeft, fActualBounds.fTop, fActualBounds.fRight, fActualBounds.fBottom);

        SkDebugf("len %f actualLen %f expectedLen %f -- actual/len = %f -- expected/len = %f\n",
                 fLength, actualLen, fExpectedLen, actualLen/fLength, fExpectedLen/fLength);
    }

private:
    void turn90(SkPoint* curPos, int turnAmount) {
        SkASSERT(turnAmount == 90 || turnAmount == -90);

        int before = fCurDir;

        fCurDir += turnAmount;
        if (fCurDir >= 360) {
            fCurDir = 0;
        } else if (fCurDir < 0) {
            fCurDir = 270;
        }

        SkASSERT(fCurDir == 0 || fCurDir == 90 || fCurDir == 180 || fCurDir == 270);
        SkDebugf("turned %d: %d -> %d\n", turnAmount, before, fCurDir);
    }

#if 0
        void curve(SkCanvas* canvas) {
            SkRect r;
            SkPaint paint;
            paint.setColor(SK_ColorRED);

            canvas->drawArc(r, 0, 360, /* useCenter= */ false, paint);
        }
#endif

    void line(SkCanvas* canvas, SkPoint* curPos, float* actualLen) {

        SkPoint before = *curPos;

        SkRect r;
        switch (fCurDir) {
            case 0:
                r.fLeft = curPos->fX;
                r.fTop = curPos->fY - fWidth / 2.0f;
                r.fRight = curPos->fX + fLength;
                r.fBottom = curPos->fY + fWidth / 2.0f;
                curPos->fX += fLength;
                break;
            case 90:
                r.fLeft = curPos->fX - fWidth / 2.0f;
                r.fTop = curPos->fY - fLength;
                r.fRight = curPos->fX + fWidth / 2.0f;
                r.fBottom = curPos->fY;
                curPos->fY -= fLength;
                break;
            case 180:
                r.fLeft = curPos->fX - fLength;
                r.fTop = curPos->fY - fWidth / 2.0f;
                r.fRight = curPos->fX;
                r.fBottom = curPos->fY + fWidth / 2.0f;
                curPos->fX -= fLength;
                break;
            case 270:
                r.fLeft = curPos->fX - fWidth / 2.0f;
                r.fTop = curPos->fY;
                r.fRight = curPos->fX + fWidth / 2.0f;
                r.fBottom = curPos->fY + fLength;
                curPos->fY += fLength;
                break;
            default:
                return;
        }

        SkPoint pts[2] = {
            before,
            *curPos,
        };

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

        SkDebugf("drawing %.2f, %.2f to %.2f, %.2f - angle: %d length: %f\n", before.fX,
                 before.fY,
                 curPos->fX, curPos->fY, fCurDir, fLength);
    }

    void recursiveDraw(SkCanvas* canvas,
                       int curDepth,
                       SkPoint* curPos,
                       int turnAmount,
                       float* actualLen) {
        if (curDepth >= fDesiredDepth) {
            return;
        }

        this->turn90(curPos, turnAmount);
        this->recursiveDraw(canvas, curDepth + 1, curPos, -turnAmount, actualLen);
        this->line(canvas, curPos, actualLen);
        this->turn90(curPos, -turnAmount);
        this->recursiveDraw(canvas, curDepth + 1, curPos, turnAmount, actualLen);
        this->line(canvas, curPos, actualLen);
        this->recursiveDraw(canvas, curDepth + 1, curPos, turnAmount, actualLen);
        this->turn90(curPos, -turnAmount);
        this->line(canvas, curPos, actualLen);
        this->recursiveDraw(canvas, curDepth + 1, curPos, -turnAmount, actualLen);
        this->turn90(curPos, turnAmount);
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
                return kColors[i]; //* t + kColors[i+1] * (1-t);
            }
        }

        return SkColors::kBlack;
    }

    float fLength;
    const int fDesiredDepth;
    const int fWidth;
    SkRect fActualBounds;
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
    static const int kLineWidth = 2;
    static const int kDepth = 4;
    static const int kPad = kLineWidth;

public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
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


        HilbertGenerator gen(kSize - kLineWidth - 2*kPad, kDepth, kLineWidth);

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
