/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/ports/SkTypeface_fontations.h"
#include "tools/Resources.h"

sk_sp<SkTypeface> fTestTypeface;
sk_sp<SkTypeface> fFtTypeface;
const SkScalar kTextSizes[] = {12, 18, 30, 36, 40, 48 };
const char kTestFontFilename[] = "fonts/Roboto-Regular.ttf";

void makeTypeface() {
    // Fontations typeface
    fTestTypeface = SkTypeface_Make_Fontations(
          GetResourceAsStream(kTestFontFilename),
          SkFontArguments());
    // FT / System typeface
    fFtTypeface =
            SkFontMgr::RefDefault()->makeFromStream(GetResourceAsStream(kTestFontFilename), 0);
    SkASSERT(fTestTypeface);
}

void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    if (!fTestTypeface) {
        return;
    }

    SkFont fontationsFont(fTestTypeface);
    SkFont ftFont(fFtTypeface);
    const char32_t testText[] = U"abc";
    size_t testTextBytesize = std::char_traits<char32_t>::length(testText) * sizeof(char32_t);
    SkScalar x = 100;
    SkScalar y = 150;

    for (SkScalar textSize : kTextSizes) {
        auto configureFont = [textSize](SkFont& font) {
            font.setSize(textSize);
            font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
            font.setSubpixel(true);
            font.setHinting(SkFontHinting::kNone);
        };
        configureFont(fontationsFont);
        configureFont(ftFont);

        canvas->drawSimpleText(
                testText, testTextBytesize, SkTextEncoding::kUTF32, x + 1 / 3.f, y, fontationsFont, paint);

        y += textSize;

        canvas->drawSimpleText(testText,
                               testTextBytesize,
                               SkTextEncoding::kUTF32,
                               x + 1 / 3.f,
                               y,
                               ftFont,
                               paint);

        y += textSize * 1.8;
    }
}

int main(int argc, char** argv) {
    SkImageInfo image_info = SkImageInfo::Make(
            800, 800, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
    const SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(image_info, &surfaceProps);
    if (!surface) return 1;
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);

    makeTypeface();
    draw(canvas);

    SkPixmap pixels;
    surface->peekPixels(&pixels);

    SkPngEncoder::Options options;
    options.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    options.fZLibLevel = 0;
    SkFILEWStream fileStream("subpixelaa.png");
    SkPngEncoder::Encode(&fileStream, pixels, options);
    return 0;
}
