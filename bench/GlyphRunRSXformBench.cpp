/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkRandom.h"
#include "tools/fonts/FontToolUtils.h"

class GlyphRunRSXformBench : public Benchmark {
public:
    GlyphRunRSXformBench() {}

protected:
    void onDelayedSetup() override {
        fFont = ToolUtils::DefaultFont();
        fFont.setSubpixel(true);
        fFont.setSize(20);

        const char* text = "Keep your sentences short, but not overly so.";
        int glyphCount = fFont.countText(text, strlen(text), SkTextEncoding::kUTF8);
        fGlyphs.append(glyphCount);
        fFont.textToGlyphs(text, strlen(text), SkTextEncoding::kUTF8, {fGlyphs.data(), (size_t)glyphCount});

        fXforms.append(glyphCount);
        SkRandom rand;
        SkScalar x = 0;
        for (int i = 0; i < glyphCount; ++i) {
            SkScalar s = rand.nextF() * 0.5f + 0.5f;
            SkScalar a = rand.nextF() * SK_ScalarPI * 0.25f;
            fXforms[i] = SkRSXform::Make(s * SkScalarCos(a), s * SkScalarSin(a), x, rand.nextF() * 20);
            x += 20;
        }
    }

    sk_sp<SkTextBlob> makeBlob() {
        return SkTextBlob::MakeFromRSXform(fGlyphs.data(), fGlyphs.size() * sizeof(SkGlyphID),
                                           {fXforms.data(), (size_t)fXforms.size()}, fFont,
                                           SkTextEncoding::kGlyphID);
    }

    SkFont fFont;
    SkTDArray<SkGlyphID> fGlyphs;
    SkTDArray<SkRSXform> fXforms;

    using INHERITED = Benchmark;
};

class GlyphRunRSXformCachedBench : public GlyphRunRSXformBench {
public:
    GlyphRunRSXformCachedBench() {}

protected:
    const char* onGetName() override { return "GlyphRunRSXform_cached"; }

    void onDelayedSetup() override {
        this->INHERITED::onDelayedSetup();
        fBlob = this->makeBlob();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(fBlob, 0, 0, paint);
        }
    }

private:
    sk_sp<SkTextBlob> fBlob;
};

class GlyphRunRSXformFirstTimeBench : public GlyphRunRSXformBench {
public:
    GlyphRunRSXformFirstTimeBench() {}

protected:
    const char* onGetName() override { return "GlyphRunRSXform_firsttime"; }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(this->makeBlob(), 0, 0, paint);
        }
    }
};

class GlyphRunRSXformMakeBench : public GlyphRunRSXformBench {
public:
    GlyphRunRSXformMakeBench() {}

protected:
    const char* onGetName() override { return "GlyphRunRSXform_make"; }

    bool isSuitableFor(Backend backend) override { return backend == Backend::kNonRendering; }

    void onDraw(int loops, SkCanvas*) override {
        for (int i = 0; i < loops; i++) {
            for (int j = 0; j < 10; ++j) {
                this->makeBlob();
            }
        }
    }
};

DEF_BENCH(return new GlyphRunRSXformCachedBench();)
DEF_BENCH(return new GlyphRunRSXformFirstTimeBench();)
DEF_BENCH(return new GlyphRunRSXformMakeBench();)
