/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "skia.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    SkPoint pts[] = {{0, 0}, {0, 100}, {100, 100}, {100, 0}, {100, 100}, {0, 100}};
  sk_sp<SkVertices> verts =
      SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, 6, pts, nullptr, nullptr);
  SkPaint paint;
  paint.setColor(SK_ColorRED);
  paint.setStyle(SkPaint::kStroke_Style);
  paint.setStrokeWidth(10);
  canvas->drawVertices(verts, SkBlendMode::kSrc, paint);


}
