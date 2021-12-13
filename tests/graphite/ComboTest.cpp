/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/private/SkCode.h"
#include "experimental/graphite/include/Context.h"
#include "tests/Test.h"

static sk_sp<SkShader> make_image_shader(int imageWidth, int imageHeight,
                                         SkTileMode xTile, SkTileMode yTile,
                                         SkColor color) {
    auto surface = SkSurface::MakeRasterN32Premul(imageWidth, imageHeight);
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(color);
    return surface->makeImageSnapshot()->makeShader(xTile, yTile, SkSamplingOptions());
}

DEF_GRAPHITE_TEST(ComboTest, r) {
    SkMagicCodeDictionary dict;

    SkCode code;

    {
        SkPaint paint;
        paint.setShader(make_image_shader(16, 16, SkTileMode::kClamp, SkTileMode::kRepeat, SK_ColorRED));
        paint.toCode(&dict, Mapping::kGanesh, &code);
        code.dump();
    }
}
