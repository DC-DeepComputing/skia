/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkCompareUtils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"

class SkPath;

bool skia_compare::A8ComparePaths(const SkPath& a, const SkPath& b, A8CompareProc cmp) {
    auto ra = a.getBounds(),
         rb = b.getBounds();

    if (ra.isEmpty() && rb.isEmpty()) {
        return true;
    }

    auto r = ra.makeOutset(1, 1);
    if (!r.contains(rb)) {
        return false;
    }

    auto ir = r.roundOut();
    const int w = ir.width(),
              h = ir.height();

    auto info = SkImageInfo::MakeA8(w, h);

    auto make_img = [&](const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);
        auto surf = SkSurfaces::Raster(info);
        auto canvas = surf->getCanvas();
        canvas->translate(1 - ir.fLeft, 1 - ir.fTop);   // keep ~1 pixel margin
        surf->getCanvas()->drawPath(a, paint);
        return surf->makeImageSnapshot();
    };
    auto imga = make_img(a),
         imgb = make_img(b);

    SkPixmap pma, pmb;
    SkAssertResult(imga->peekPixels(&pma));
    SkAssertResult(imgb->peekPixels(&pmb));

    for (int y = 0; y < pma.height(); ++y) {
        for (int x = 0; x < pma.width(); ++x) {
            uint8_t pa = *pma.addr8(x, y),
                    pb = *pmb.addr8(x, y);
            if (pa != pb) {
                if (!cmp(x, y, pa, pb)) {
                    return false;
                }
            }
        }
    }
    return true;
}
