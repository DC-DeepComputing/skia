/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTDArray.h"
#include "include/private/chromium/Slug.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/ContextOptions.h"
#endif

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrContextOptions.h"
#endif

#if defined(SK_GANESH) || defined(SK_GRAPHITE)

class SlugGM : public skiagm::GM {
public:
    SlugGM(const char* txt) : fText(txt) {}

protected:
#if defined(SK_GANESH)
    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        ctxOptions->fSupportBilerpFromGlyphAtlas = true;
    }
#endif

#if defined(SK_GRAPHITE)
    void modifyGraphiteContextOptions(skgpu::graphite::ContextOptions* options) const override {
        options->fSupportBilerpFromGlyphAtlas = true;
    }
#endif

    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
        SkFont font(fTypeface);
        size_t txtLen = strlen(fText);
        int glyphCount = font.countText(fText, txtLen, SkTextEncoding::kUTF8);

        fGlyphs.append(glyphCount);
        font.textToGlyphs(fText, txtLen, SkTextEncoding::kUTF8, fGlyphs);
    }

    SkString getName() const override { return SkString("slug"); }

    SkISize getISize() override { return SkISize::Make(1000, 480); }

    void onDraw(SkCanvas* canvas) override {
        sk_sp<SkTextBlob> blob(this->makeBlob());
        SkPaint p;
        p.setAntiAlias(true);
        canvas->clipIRect(SkIRect::MakeSize(this->getISize()).makeInset(40, 50));
        canvas->scale(1.3f, 1.3f);
        sk_sp<sktext::gpu::Slug> slug = sktext::gpu::Slug::ConvertBlob(canvas, *blob, {10, 10}, p);
        if (slug == nullptr) {
            return;
        }
        canvas->translate(0.5, 0.5);
        canvas->translate(30, 30);
        canvas->drawTextBlob(blob, 10, 10, p);
        canvas->translate(370, 0);
        slug->draw(canvas, p);
        for (float scale = 1.5; scale < 4; scale += 0.5) {
            canvas->translate(-370, 20 * scale);
            canvas->save();
            canvas->scale(scale, scale);
            canvas->rotate(5);
            canvas->drawTextBlob(blob, 10, 10, p);
            canvas->restore();
            canvas->translate(370, 0);
            canvas->save();
            canvas->scale(scale, scale);
            canvas->rotate(5);

            slug->draw(canvas, p);
            canvas->restore();
        }
    }

private:
    sk_sp<SkTextBlob> makeBlob() {
        SkTextBlobBuilder builder;

        SkFont font;
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setTypeface(fTypeface);
        font.setSize(16);

        const SkTextBlobBuilder::RunBuffer& buf = builder.allocRun(font, fGlyphs.size(), 0, 0);
        memcpy(buf.glyphs, fGlyphs.begin(), fGlyphs.size() * sizeof(uint16_t));
        return builder.make();
    }

    SkTDArray<SkGlyphID> fGlyphs;
    sk_sp<SkTypeface>   fTypeface;
    const char*         fText;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new SlugGM("hamburgefons");)

// Exercises SDFTSubRun inside a Slug by converting text blobs under conditions where
// SubRunControl::isSDFT() is true:
// 1. Large device text size in [kLargeDFFontLimit (162), maxMaskSize (256)), drawn at multiple
//    scales within its SDFTMatrixRange.
// 2. Perspective CTM (matrix.hasPerspective() == true).
class SlugSDFTGM : public skiagm::GM {
public:
    SlugSDFTGM() = default;

protected:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
    }

    SkString getName() const override { return SkString("slug_sdft"); }

    SkISize getISize() override { return SkISize::Make(1000, 600); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        // 1. Large text that falls into the SDFT size range [162, 256).
        // Font size 64 at canvas scale 2.6 -> approximateDeviceTextSize = 166.4 -> SDFTSubRun.
        {
            SkFont font(fTypeface, 64);
            font.setEdging(SkFont::Edging::kAntiAlias);
            sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("SDFT", font);

            canvas->save();
            canvas->translate(30, 180);
            canvas->scale(2.6f, 2.6f);
            sk_sp<sktext::gpu::Slug> slug =
                    sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
            canvas->drawTextBlob(blob, 0, 0, paint);
            canvas->restore();

            if (slug) {
                // Draw the SDFT slug at the creation scale on the right.
                canvas->save();
                canvas->translate(500, 180);
                canvas->scale(2.6f, 2.6f);
                slug->draw(canvas, paint);
                canvas->restore();

                // Draw again at a slightly different scale within the SDFTMatrixRange to
                // exercise SDFT distance-field scaling from a single cached Slug.
                canvas->save();
                canvas->translate(30, 360);
                canvas->scale(3.1f, 3.1f);
                canvas->drawTextBlob(blob, 0, 0, paint);
                canvas->restore();

                canvas->save();
                canvas->translate(500, 360);
                canvas->scale(3.1f, 3.1f);
                slug->draw(canvas, paint);
                canvas->restore();
            }
        }

        // 2. Perspective CTM, which always selects SDFTSubRun for outline glyphs.
        {
            SkFont font(fTypeface, 44);
            font.setEdging(SkFont::Edging::kAntiAlias);
            sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromString("Perspective SDFT", font);

            SkMatrix persp = SkMatrix::I();
            persp.setPerspX(0.0004f);
            persp.setPerspY(0.0008f);

            canvas->save();
            canvas->translate(30, 500);
            canvas->concat(persp);
            sk_sp<sktext::gpu::Slug> slug =
                    sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
            canvas->drawTextBlob(blob, 0, 0, paint);
            canvas->restore();

            if (slug) {
                canvas->save();
                canvas->translate(500, 500);
                canvas->concat(persp);
                slug->draw(canvas, paint);
                canvas->restore();
            }
        }
    }

private:
    sk_sp<SkTypeface> fTypeface;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new SlugSDFTGM();)

// Exercises DrawableSubRun inside a Slug using COLRv1 color glyphs under conditions where
// DirectMaskSubRun is bypassed (large device text size >= 256, and perspective CTM), causing
// SubRunContainer::MakeInAlloc to fall through to DrawableSubRun.
class SlugDrawableGM : public skiagm::GM {
public:
    SlugDrawableGM() = default;

protected:
    void onOnceBeforeDraw() override {
        fTypeface = ToolUtils::CreateTypefaceFromResource("fonts/test_glyphs-glyf_colr_1.ttf");
    }

    SkString getName() const override { return SkString("slug_drawable"); }

    SkISize getISize() override { return SkISize::Make(1000, 650); }

    void onDraw(SkCanvas* canvas) override {
        if (!fTypeface) {
            return;
        }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLACK);

        // COLRv1 test codepoints (gradient stops & sweep gradients from test_glyphs-glyf_colr_1.ttf)
        const uint32_t codepoints[] = {0xf0100, 0xf0101, 0xf0201};

        // 1. Large COLRv1 glyphs (device text size 280 >= maxMaskSize 256) -> DrawableSubRun.
        {
            SkFont font(fTypeface, 140);
            font.setEdging(SkFont::Edging::kAntiAlias);
            sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(
                    codepoints, sizeof(codepoints), font, SkTextEncoding::kUTF32);

            canvas->save();
            canvas->translate(30, 300);
            canvas->scale(2.0f, 2.0f);
            sk_sp<sktext::gpu::Slug> slug =
                    sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
            canvas->scale(0.35f, 0.35f);
            canvas->drawTextBlob(blob, 0, 0, paint);
            canvas->restore();

            if (slug) {
                canvas->save();
                canvas->translate(500, 300);
                canvas->scale(0.7f, 0.7f);
                slug->draw(canvas, paint);
                canvas->restore();
            }
        }

        // 2. Perspective-transformed COLRv1 glyphs -> rejected by SDFTSubRun (not kSDF_Format),
        // skipped by DirectMaskSubRun (hasPerspective()), accepted by DrawableSubRun.
        {
            SkFont font(fTypeface, 90);
            font.setEdging(SkFont::Edging::kAntiAlias);
            sk_sp<SkTextBlob> blob = SkTextBlob::MakeFromText(
                    codepoints, sizeof(codepoints), font, SkTextEncoding::kUTF32);

            SkMatrix persp = SkMatrix::I();
            persp.setPerspX(0.0005f);
            persp.setPerspY(0.0008f);

            canvas->save();
            canvas->translate(30, 540);
            canvas->concat(persp);
            sk_sp<sktext::gpu::Slug> slug =
                    sktext::gpu::Slug::ConvertBlob(canvas, *blob, {0, 0}, paint);
            canvas->drawTextBlob(blob, 0, 0, paint);
            canvas->restore();

            if (slug) {
                canvas->save();
                canvas->translate(500, 540);
                canvas->concat(persp);
                slug->draw(canvas, paint);
                canvas->restore();
            }
        }
    }

private:
    sk_sp<SkTypeface> fTypeface;
    using INHERITED = skiagm::GM;
};

DEF_GM(return new SlugDrawableGM();)
#endif
