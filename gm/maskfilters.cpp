/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkTableMaskFilter.h"

DEF_SIMPLE_GM(tableMaskFilter, canvas, 400, 400) {
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorBLACK);

    uint8_t table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = 255 / 2;
    }
    table[255] = 255;
    paint.setMaskFilter(sk_sp<SkMaskFilter>(SkTableMaskFilter::Create(table)));

    SkPath path;
    path.addRect({38.f, 38.f, 218.f, 218.f});
    path.addOval({38.f, 38.f, 218.f, 218.f}, SkPathDirection::kCCW);

    canvas->drawPath(path, paint);
  }
