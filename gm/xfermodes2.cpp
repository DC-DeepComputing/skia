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
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <stdint.h>
#include <string.h>

static sk_sp<SkBlender> dst_blender() {
    auto [effect, error] = SkRuntimeEffect::MakeForBlender(SkString(R"(
half guarded_divide(half n, half d) {
    return n / d;
}

half color_dodge_component(half2 s, half2 d) {
    if (d.x == 0) {
        return s.x*(1 - d.y);
    } else {
        half delta = s.y - s.x;
        if (delta == 0) {
            return s.y*d.y + s.x*(1 - d.y) + d.x*(1 - s.y);
        } else {
            delta = min(d.y, guarded_divide(d.x*s.y, delta));
            return delta*s.y + s.x*(1 - d.y) + d.x*(1 - s.y);
        }
    }
}

half4 blend_color_dodge(half4 src, half4 dst) {
    return half4(color_dodge_component(src.ra, dst.ra),
                 color_dodge_component(src.ga, dst.ga),
                 color_dodge_component(src.ba, dst.ba),
                 src.a + (1 - src.a)*dst.a);
}


half color_burn_component(half2 s, half2 d) {
    if (d.y == d.x) {
        return s.y*d.y + s.x*(1 - d.y) + d.x*(1 - s.y);
    } else if (s.x == 0) {
        return d.x*(1 - s.y);
    } else {
        half delta = max(0, d.y - guarded_divide((d.y - d.x)*s.y, s.x));
        return delta*s.y + s.x*(1 - d.y) + d.x*(1 - s.y);
    }
}

half4 blend_color_burn(half4 src, half4 dst) {
    return half4(color_burn_component(src.ra, dst.ra),
                 color_burn_component(src.ga, dst.ga),
                 color_burn_component(src.ba, dst.ba),
                 src.a + (1 - src.a)*dst.a);
}

        half4 main(half4 src, half4 dst) {
            return blend_color_dodge(src, dst);
        }
    )"));
    SkASSERT(effect);
    return effect->makeBlender(nullptr);
}

namespace skiagm {

class Xfermodes2GM : public GM {
public:
    Xfermodes2GM(bool grayscale) : fGrayscale(grayscale) {}

protected:
    SkString getName() const override {
        return fGrayscale ? SkString("xfermodes2_gray") : SkString("xfermodes2");
    }

    SkISize getISize() override { return SkISize::Make(455, 475); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkScalar w = SkIntToScalar(kSize);
        const SkScalar h = SkIntToScalar(kSize);

        SkFont font = ToolUtils::DefaultPortableFont();

        SkScalar x = 0, y = 0;
        for (SkBlendMode mode : {SkBlendMode::kColorBurn, SkBlendMode::kColorDodge, SkBlendMode::kColor}) {
            canvas->save();

            canvas->translate(x, y);
            SkPaint p;
            p.setAntiAlias(false);
            p.setStyle(SkPaint::kFill_Style);
            p.setShader(fBG);
            SkRect r = SkRect::MakeWH(w, h);
            canvas->drawRect(r, p);

            canvas->saveLayer(&r, nullptr);

            p.setShader(fDst);
            canvas->drawRect(r, p);
            p.setShader(fSrc);
            if (mode == SkBlendMode::kColor) {
                p.setBlender(dst_blender());
            } else {
                p.setBlendMode(mode);
            }
            canvas->drawRect(r, p);

            canvas->restore();

            r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
            p.setStyle(SkPaint::kStroke_Style);
            p.setShader(nullptr);
            p.setBlendMode(SkBlendMode::kSrcOver);
            canvas->drawRect(r, p);

            canvas->restore();

#if 1
            SkTextUtils::DrawString(canvas, SkBlendMode_Name(mode), x + w/2, y - font.getSize()/2, font, SkPaint(),
                                    SkTextUtils::kCenter_Align);
#endif
            x += w + SkIntToScalar(10);
        }
    }

private:
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
        lm.setScale(SkIntToScalar(16), SkIntToScalar(16));
        fBG = bg.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions(), lm);

        SkBitmap srcBmp;
        srcBmp.allocN32Pixels(kSize, kSize);
        SkPMColor* pixels = reinterpret_cast<SkPMColor*>(srcBmp.getPixels());

        for (int y = 0; y < kSize; ++y) {
            int c = y * (1 << kShift);
            SkPMColor rowColor = fGrayscale ? SkPackARGB32(c, c, c, c) : SkPackARGB32(c, c, 0, c/2);
            for (int x = 0; x < kSize; ++x) {
                pixels[kSize * y + x] = rowColor;
            }
        }
        fSrc = srcBmp.makeShader(SkSamplingOptions());
        SkBitmap dstBmp;
        dstBmp.allocN32Pixels(kSize, kSize);
        pixels = reinterpret_cast<SkPMColor*>(dstBmp.getPixels());

        for (int x = 0; x < kSize; ++x) {
            int c = x * (1 << kShift);
            SkPMColor colColor = fGrayscale ? SkPackARGB32(c, c, c, c) : SkPackARGB32(c, 0, c, c/2);
            for (int y = 0; y < kSize; ++y) {
                pixels[kSize * y + x] = colColor;
            }
        }
        fDst = dstBmp.makeShader(SkSamplingOptions());
    }

    enum {
        kShift = 2,
        kSize = 256 >> kShift,
    };

    bool fGrayscale;
    sk_sp<SkShader> fBG;
    sk_sp<SkShader> fSrc;
    sk_sp<SkShader> fDst;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new Xfermodes2GM(false); )
DEF_GM( return new Xfermodes2GM(true); )

}  // namespace skiagm
