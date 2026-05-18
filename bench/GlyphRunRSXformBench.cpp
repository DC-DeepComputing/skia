/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkCustomTypeface.h"

namespace {

sk_sp<SkTypeface> MakePathTypeface() {
    SkCustomTypefaceBuilder builder;
    SkFontMetrics metrics;
    metrics.fTop = -1.0f;
    metrics.fAscent = -0.8f;
    metrics.fDescent = 0.2f;
    metrics.fBottom = 0.3f;
    metrics.fXMin = 0.0f;
    metrics.fXMax = 1.0f;
    builder.setMetrics(metrics);
    builder.setGlyph(0, 1.0f, SkPath::Rect(SkRect::MakeLTRB(0.1f, -0.8f, 0.9f, 0.0f)));
    return builder.detach();
}

sk_sp<SkTextBlob> MakeInnerBlob(sk_sp<SkTypeface> tf) {
    SkTextBlobBuilder builder;
    for (int i = 0; i < 64; ++i) {
        SkFont font(tf, 8.0f + static_cast<float>(i));
        const auto& run = builder.allocRunPos(font, 1);
        run.glyphs[0] = 0;
        run.points()[0] = SkPoint::Make(0, 0);
    }
    return builder.make();
}

class ReentrantDrawable final : public SkDrawable {
public:
    explicit ReentrantDrawable(sk_sp<SkTextBlob> inner) : fInner(std::move(inner)) {}
    void reset() { fFired = false; }
protected:
    SkRect onGetBounds() override { return SkRect::MakeLTRB(0, -1, 1, 0); }
    void onDraw(SkCanvas* canvas) override {
        if (fFired) { return; }
        fFired = true;
        SkPaint paint;
        canvas->drawTextBlob(fInner, 0, 0, paint);
    }
private:
    sk_sp<SkTextBlob> fInner;
    bool fFired = false;
};

sk_sp<SkTypeface> MakeDrawableTypeface(sk_sp<SkDrawable> drawable) {
    SkCustomTypefaceBuilder builder;
    SkFontMetrics metrics;
    metrics.fTop = -1.0f;
    metrics.fAscent = -0.8f;
    metrics.fDescent = 0.2f;
    metrics.fBottom = 0.3f;
    metrics.fXMin = 0.0f;
    metrics.fXMax = 1.0f;
    builder.setMetrics(metrics);
    builder.setGlyph(0, 1.0f, std::move(drawable), SkRect::MakeLTRB(0, -1, 1, 0));
    return builder.detach();
}

sk_sp<SkTextBlob> MakeOuterBlob(sk_sp<SkTypeface> drawable_tf, sk_sp<SkTypeface> path_tf) {
    SkTextBlobBuilder builder;
    SkFont font0(std::move(drawable_tf), 400.0f);
    {
        const auto& run = builder.allocRunPos(font0, 1);
        run.glyphs[0] = 0;
        run.points()[0] = SkPoint::Make(0, 0);
    }
    SkFont font1(std::move(path_tf), 20.0f);
    {
        const auto& run = builder.allocRunRSXform(font1, 1);
        run.glyphs[0] = 0;
        run.xforms()[0] = SkRSXform::Make(1, 0, 0, 0);
    }
    return builder.make();
}

}  // namespace

class GlyphRunUAFBench : public Benchmark {
public:
    GlyphRunUAFBench() {}

protected:
    const char* onGetName() override { return "GlyphRunUAFBench"; }

    void onDelayedSetup() override {
        fPathTf = MakePathTypeface();
        sk_sp<SkTextBlob> inner = MakeInnerBlob(fPathTf);
        fDrawable = sk_make_sp<ReentrantDrawable>(std::move(inner));
        sk_sp<SkTypeface> drawable_tf = MakeDrawableTypeface(fDrawable);
        fOuterBlob = MakeOuterBlob(std::move(drawable_tf), fPathTf);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            fDrawable->reset();
            canvas->drawTextBlob(fOuterBlob, 100, 500, paint);
        }
    }

private:
    sk_sp<SkTypeface> fPathTf;
    sk_sp<ReentrantDrawable> fDrawable;
    sk_sp<SkTextBlob> fOuterBlob;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new GlyphRunUAFBench(); )

class NormalRSXformBench : public Benchmark {
public:
    NormalRSXformBench() {}

protected:
    const char* onGetName() override { return "NormalRSXformBench"; }

    void onDelayedSetup() override {
        fPathTf = MakePathTypeface();
        SkTextBlobBuilder builder;
        SkFont font(fPathTf, 20.0f);
        const auto& run = builder.allocRunRSXform(font, 100);
        for (int i = 0; i < 100; ++i) {
            run.glyphs[i] = 0;
            run.xforms()[i] = SkRSXform::Make(1, 0, i * 20.0f, 0);
        }
        fBlob = builder.make();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            canvas->drawTextBlob(fBlob, 0, 0, paint);
        }
    }

private:
    sk_sp<SkTypeface> fPathTf;
    sk_sp<SkTextBlob> fBlob;

    using INHERITED = Benchmark;
};

DEF_BENCH( return new NormalRSXformBench(); )
