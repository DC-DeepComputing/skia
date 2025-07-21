/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkColorPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

namespace skiagm {

/**
 * This tests drawing device-covering rects with solid colors and bitmap shaders over a
 * checkerboard background using different xfermodes.
 */
class Xfermodes3GM : public GM {
public:
    Xfermodes3GM() { this->setBGColor(ToolUtils::color_to_565(0xFF70D0E0)); }

protected:
    SkString getName() const override { return SkString("xfermodes3"); }

    SkISize getISize() override { return SkISize::Make(630, 1215); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        SkFont  font = ToolUtils::DefaultPortableFont();
        SkPaint labelP;

        constexpr SkColor kSolidColors[] = {
            SK_ColorTRANSPARENT,
            SK_ColorBLUE,
            0x80808000
        };

        constexpr SkColor kBmpAlphas[] = {
            0xff,
            0x80,
        };

        auto tempSurface(this->makeTempSurface(canvas, kSize, kSize));

        int test = 0;
        int x = 0, y = 0;
        constexpr struct { SkPaint::Style fStyle; SkScalar fWidth; } kStrokes[] = {
            {SkPaint::kFill_Style, 0},
            {SkPaint::kStroke_Style, SkIntToScalar(kSize) / 2},
        };
        for (size_t s = 0; s < std::size(kStrokes); ++s) {
            for (size_t m = 0; m < kSkBlendModeCount; ++m) {
                SkBlendMode mode = static_cast<SkBlendMode>(m);
                canvas->drawString(SkBlendMode_Name(mode),
                                   SkIntToScalar(x),
                                   SkIntToScalar(y + kSize + 3) + font.getSize(),
                                   font, labelP);
                for (size_t c = 0; c < std::size(kSolidColors); ++c) {
                    SkPaint modePaint;
                    modePaint.setBlendMode(mode);
                    modePaint.setColor(kSolidColors[c]);
                    modePaint.setStyle(kStrokes[s].fStyle);
                    modePaint.setStrokeWidth(kStrokes[s].fWidth);

                    this->drawMode(canvas, x, y, kSize, kSize, modePaint, tempSurface.get());

                    ++test;
                    x += kSize + 10;
                    if (!(test % kTestsPerRow)) {
                        x = 0;
                        y += kSize + 30;
                    }
                }
                for (size_t a = 0; a < std::size(kBmpAlphas); ++a) {
                    SkPaint modePaint;
                    modePaint.setBlendMode(mode);
                    modePaint.setAlpha(kBmpAlphas[a]);
                    modePaint.setShader(fBmpShader);
                    modePaint.setStyle(kStrokes[s].fStyle);
                    modePaint.setStrokeWidth(kStrokes[s].fWidth);

                    this->drawMode(canvas, x, y, kSize, kSize, modePaint, tempSurface.get());

                    ++test;
                    x += kSize + 10;
                    if (!(test % kTestsPerRow)) {
                        x = 0;
                        y += kSize + 30;
                    }
                }
            }
        }
    }

private:
    /**
     * GrContext has optimizations around full rendertarget draws that can be replaced with clears.
     * We are trying to test those. We could use saveLayer() to create small SkGpuDevices but
     * saveLayer() uses the texture cache. This means that the actual render target may be larger
     * than the layer. Because the clip will contain the layer's bounds, no draws will be full-RT.
     * So explicitly create a temporary canvas with dimensions exactly the layer size.
     */
    sk_sp<SkSurface> makeTempSurface(SkCanvas* baseCanvas, int w, int h) {
        SkImageInfo baseInfo = baseCanvas->imageInfo();
        SkImageInfo info = SkImageInfo::Make(w, h, baseInfo.colorType(), baseInfo.alphaType(),
                                             baseInfo.refColorSpace());
        return baseCanvas->makeSurface(info);
    }

    void drawMode(SkCanvas* canvas,
                  int x, int y, int w, int h,
                  const SkPaint& modePaint, SkSurface* surface) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));

        SkRect r = SkRect::MakeWH(SkIntToScalar(w), SkIntToScalar(h));

        SkCanvas* modeCanvas;
        if (nullptr == surface) {
            canvas->saveLayer(&r, nullptr);
            canvas->clipRect(r);
            modeCanvas = canvas;
        } else {
            modeCanvas = surface->getCanvas();
        }

        SkPaint bgPaint;
        bgPaint.setAntiAlias(false);
        bgPaint.setShader(fBGShader);
        modeCanvas->drawRect(r, bgPaint);
        modeCanvas->drawRect(r, modePaint);
        modeCanvas = nullptr;

        if (nullptr == surface) {
            canvas->restore();
        } else {
            surface->draw(canvas, 0, 0);
        }

        r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        SkPaint borderPaint;
        borderPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, borderPaint);

        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        const uint32_t kCheckData[] = {
            SkPackARGB32(0xFF, 0x42, 0x41, 0x42),
            SkPackARGB32(0xFF, 0xD6, 0xD3, 0xD6),
            SkPackARGB32(0xFF, 0xD6, 0xD3, 0xD6),
            SkPackARGB32(0xFF, 0x42, 0x41, 0x42)
        };
        SkBitmap bg;
        bg.allocN32Pixels(2, 2, true);
        memcpy(bg.getPixels(), kCheckData, sizeof(kCheckData));

        SkMatrix lm;
        lm.setScale(SkIntToScalar(kCheckSize), SkIntToScalar(kCheckSize));
        fBGShader = bg.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                  SkSamplingOptions(), lm);

        SkPaint bmpPaint;
        const SkPoint kCenter = { SkIntToScalar(kSize) / 2, SkIntToScalar(kSize) / 2 };
        const SkColor kColors[] = {
            SK_ColorTRANSPARENT, 0x80800000, 0xF020F060, SK_ColorWHITE
        };
        bmpPaint.setShader(SkGradientShader::MakeRadial(kCenter, 3 * SkIntToScalar(kSize) / 4,
                                                        kColors, nullptr, std::size(kColors),
                                                        SkTileMode::kRepeat));

        SkBitmap bmp;
        bmp.allocN32Pixels(kSize, kSize);
        SkCanvas bmpCanvas(bmp);

        bmpCanvas.clear(SK_ColorTRANSPARENT);
        SkRect rect = { SkIntToScalar(kSize) / 8, SkIntToScalar(kSize) / 8,
                        7 * SkIntToScalar(kSize) / 8, 7 * SkIntToScalar(kSize) / 8};
        bmpCanvas.drawRect(rect, bmpPaint);

        fBmpShader = bmp.makeShader(SkSamplingOptions());
    }

    enum {
        kCheckSize = 8,
        kSize = 30,
        kTestsPerRow = 15,
    };

    sk_sp<SkShader> fBGShader;
    sk_sp<SkShader> fBmpShader;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new Xfermodes3GM;)

// This GM tests the continuity of the burn/dodge functions when they have
// src == 0 or 1, and when dst == 0 or 1, which cause certain denominators in
// main equation to become 0.
DEF_SIMPLE_GM(burn_dodge_continuity, canvas, 360, 219) {
    // This image has pixels where a color component equals its alpha channel (1) and pixels that
    // are very close to alpha = 1 causing them to take different branches based on d == da.
    auto image = ToolUtils::GetResourceAsImage("images/burn_blend_background.jpg");

    auto drawBlend = [&](SkBlendMode mode) {
        canvas->save();
            canvas->clipRect(SkRect::MakeLTRB(0, 0, image->width(), image->height() / 2));
            canvas->drawImage(image, 0, 0);
        canvas->restore();
        // For dodge, we want the opposite situation, where d approaches 0, so draw an inverted
        // image to produce colors that will be tricky for dodge.
        canvas->save();
            canvas->clipRect(SkRect::MakeLTRB(0, image->height()/2,
                                              image->width(), image->height()));
            canvas->scale(1.f, -1.f);
            canvas->clear(SK_ColorWHITE);
            SkPaint sub;
            sub.setBlendMode(SkBlendMode::kExclusion);
            canvas->drawImage(image, 0, -image->height(), {}, &sub);
        canvas->restore();

        SkRect imageRect = SkRect::Make(image->bounds());
        SkRect blendRect = imageRect.makeInset(32.f, 32.f);

        // Also draw a gradient from black to white to test the behavior of burn/dodge.
        SkPoint pts[2] = {{0.f, 64.f}, {0.f, image->height() - 64.f}};
        SkColor colors[2] = {SK_ColorBLACK, SK_ColorWHITE};
        sk_sp<SkShader> gradient =
                SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
        SkPaint srcGrad;
        srcGrad.setBlendMode(SkBlendMode::kSrc);
        srcGrad.setShader(gradient);
        // srcGrad.setAlpha(0x75);
        canvas->drawRect(SkRect::MakeLTRB(imageRect.centerX(),
                                          imageRect.top(),
                                          imageRect.right(),
                                          imageRect.bottom()),
                         srcGrad);

        // Draw four alternating rectangles covering the image and the gradient, going
        // black, white, black, white.  For burn, white is a no-op. For dodge, black is a no-op.
        SkPaint p;
        p.setColor(colors[0]);
        p.setAlphaf(0.5f);
        p.setBlendMode(mode);
        canvas->drawRect(SkRect::MakeLTRB(blendRect.left(),
                                          blendRect.top(),
                                          0.5f * (blendRect.left() + blendRect.centerX()),
                                          blendRect.bottom()), p);
        canvas->drawRect(SkRect::MakeLTRB(blendRect.centerX(),
                                          blendRect.top(),
                                          0.5f * (blendRect.centerX() + blendRect.right()),
                                          blendRect.bottom()), p);

        p.setColor(colors[1]);
        canvas->drawRect(SkRect::MakeLTRB(0.5f * (blendRect.left() + blendRect.centerX()),
                                          blendRect.top(),
                                          blendRect.centerX(),
                                          blendRect.bottom()), p);
        canvas->drawRect(SkRect::MakeLTRB(0.5f * (blendRect.centerX() + blendRect.right()),
                                          blendRect.top(),
                                          blendRect.right(),
                                          blendRect.bottom()), p);

        SkPaint line;
        line.setColor(SK_ColorRED);
        line.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(blendRect, line);
    };

    canvas->scale(0.25f, 0.25f);
    drawBlend(SkBlendMode::kColorBurn);
    canvas->translate(image->width(), 0.f);
    drawBlend(SkBlendMode::kColorDodge);
}

}  // namespace skiagm
