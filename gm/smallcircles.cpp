/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkImageFilters.h"

static void draw_small_circle(SkCanvas* canvas, const SkPaint& p, const float radius) {
    SkRect oval = SkRect::MakeLTRB(radius * -1, radius * -1, radius, radius);
    bool useCenter = true;
    float degrees = 360;
    canvas->drawArc(oval, 0, degrees, useCenter, p);
}

DEF_SIMPLE_GM(smallcircles, canvas, 500, 500) {
        auto surface = canvas->makeSurface(SkImageInfo::MakeN32Premul(500, 500));
        if (!surface) {
          surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(500, 500));
        }
        SkCanvas* canv = surface->getCanvas();

        canv->translate(5, 5);
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kFill_Style);

        // Draw circles
        for (int i = 0; i < 11; ++i) {
          canv->save();
          for (int j = 0; j < 11; ++j) {
            draw_small_circle(canv, p, 0.8);
            canv->translate(5.1, 0);
          }
          canv->restore();
          canv->translate(0, 5.1);
        }

        // Scale up image of small circles
        canvas->scale(10, 10);
        auto img = surface->makeImageSnapshot();
        canvas->drawImage(img, 0, 0);
}
