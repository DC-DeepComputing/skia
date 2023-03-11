/*
 * Copyright 2023 Google Inc.
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

static void draw_mode(SkPixmap pm, SkBlendMode mode, const SkPath& path, SkAlpha alpha, bool aa) {
    SkPaint paint;
    paint.setBlendMode(mode);
    paint.setAlpha(alpha);
    paint.setAntiAlias(aa);

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

    auto draw_set = [&](bool aa) {
        const SkBlendMode modes[] = {
            SkBlendMode::kDstOut,
            SkBlendMode::kModulate,
            SkBlendMode::kSrcOver,
            SkBlendMode::kXor,
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
        };

        const SkAlpha alphas[] = {0, 0x60, 0xFF};

        float x = 0;
        for (auto m : modes) {
            float y = 0;
            for (auto a : alphas) {
                do_setup(surf->getCanvas());
                draw_mode(pm, m, path, a, aa);
                auto img = surf->makeImageSnapshot();
                canvas->drawImage(img, x, y);
                y += N;
            }
            x += N;
        }
    };
    draw_set(false);
    canvas->translate(0, N*3 + 10);
    draw_set(true);
}
