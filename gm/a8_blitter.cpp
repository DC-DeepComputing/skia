/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"

#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterClip.h"

const int N = 128;

static void do_setup(SkCanvas* canvas) {
    canvas->clear(0);

    const SkPoint pts[] = {
        {0,0}, {N,N}, {0,N}
    };
    canvas->drawPath(SkPath::Polygon(pts, 3, true), SkPaint());
}

static void draw_mode(const SkPixmap& pm, const SkPath& path, const SkPaint& paint) {
    SkRasterClip rclip({0, 0, N, N});
    SkMatrixProvider mprovider(SkMatrix::I());

    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst            = pm;
    draw.fRC             = &rclip;
    draw.fMatrixProvider = &mprovider;

    draw.drawPath(path, paint);
}

DEF_SIMPLE_GM(a8_blitter, canvas, 1792, 1080) {
    auto path = SkPath::Circle(N/2, N/2, N/3);

    SkPaint paint;

    const auto info = SkImageInfo::Make(N, N, kAlpha_8_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRaster(info, N, nullptr);
    SkPixmap pm;
    bool success = surf->peekPixels(&pm);
    assert(success);

    auto draw_one = [&](SkBlendMode mode, SkAlpha alpha, bool aa) {
        SkPaint paint;
        paint.setBlendMode(mode);
        paint.setAlpha(alpha);
        paint.setAntiAlias(aa);

        do_setup(surf->getCanvas());
        draw_mode(pm, path, paint);
        auto img = surf->makeImageSnapshot();
        canvas->drawImage(img, 0, 0);
    };

    auto draw_set = [&](SkBlendMode mode) {
        const SkAlpha alphas[] = {0, 0x60, 0xFF};

        SkPaint paint;
        paint.setBlendMode(mode);

        canvas->save();
        for (auto a : alphas) {
            draw_one(mode, a, false);
            canvas->translate(N, 0);
            draw_one(mode, a, true);
            canvas->translate(N + N/8, 0);
        }
        canvas->restore();
    };

    const SkBlendMode modes[] = {
        SkBlendMode::kDstOut,
        SkBlendMode::kModulate,
        SkBlendMode::kSrcOver,
        SkBlendMode::kXor,
        SkBlendMode::kClear,
        SkBlendMode::kSrc,
    };
    for (auto m : modes) {
        draw_set(m);
        canvas->translate(0, N + N/8);
    }
}
