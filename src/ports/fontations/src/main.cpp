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
    auto typeface = sk_sp<SkTypeface>(new SkTypeface_Fontations(std::move(font_data)));
    printf("Glyph id for U+FE003: %d\n", typeface->unicharToGlyph(0xFE003));
    return 0;
}
