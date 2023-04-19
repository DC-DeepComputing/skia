/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypeface.h"
#include "src/ports/SkTypeface_fontations.h"
#include "tools/Resources.h"

namespace {
const char kTestFontName[] = "fonts/test_glyphs-glyf_colr_1.ttf";

}  // namespace

int main(int argc, char** argv) {
    std::unique_ptr<SkStreamAsset> font_data = GetResourceAsStream(kTestFontName);
    sk_sp<SkTypeface> typeface(new SkTypeface_Fontations(std::move(font_data)));
    SkASSERT(typeface.get());
    printf("Glyph id for U+FE003: %d\n", typeface->unicharToGlyph(0xFE003));

    SkImageInfo image_info = SkImageInfo::Make(
            800, 1200, SkColorType::kRGB_565_SkColorType, SkAlphaType::kOpaque_SkAlphaType);
    sk_sp<SkSurface> surface(SkSurface::MakeRaster(image_info));
    if (!surface) return 1;
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;

    const char32_t testtext[] = U"abc";
    size_t testtext_bytesize = sizeof(testtext) / sizeof(char32_t) * sizeof(char32_t);

    SkScalar horizontal_offset = 170;
    SkScalar line_increment;
    SkScalar first_block_y = 20;
    SkScalar second_block_y = 600;
    for (float font_size = 11; font_size < 20.2; font_size += (1 / 3.0f)) {
        SkFont font(typeface);
        font.setSize(font_size);
        line_increment += font_size * 1.3;
        canvas->drawSimpleText(testtext,
                               testtext_bytesize,
                               SkTextEncoding::kUTF32,
                               horizontal_offset,
                               first_block_y + line_increment,
                               font,
                               paint);
    }

    sk_sp<SkImage> image = surface->makeImageSnapshot();
    sk_sp<SkData> png = image->encodeToData(SkEncodedImageFormat::kPNG, 100);
    SkFILEWStream{"fontations.png"}.write(png->data(), png->size());

    return 0;
}
