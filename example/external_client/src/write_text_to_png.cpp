/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkRRect.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include "include/effects/SkGradientShader.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>", argv[0]);
        return 1;
    }
    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }
    sk_sp<SkSurface> surface = SkSurfaces::Raster(SkImageInfo::MakeN32(100, 50, kOpaque_SkAlphaType));
    SkCanvas* canvas = surface->getCanvas();
#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());
#endif
#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
#endif

    sk_sp<SkTypeface> face = mgr->matchFamilyStyle("Roboto", SkFontStyle());
    if (!face) {
      printf("Cannot open typeface\n");
      return 1;
    }
    SkFont font(face, 14);
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    canvas->clear(SK_ColorYELLOW);
    canvas->drawString("Hello world!", 10, 25, font, paint);

    {
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        //paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0));
        SkColor colors[] = {
            SkColorSetRGB(0xE4, 0x03, 0x03), // Red
            SkColorSetRGB(0xFF, 0x8C, 0x00), // Orange
            SkColorSetRGB(0xFF, 0xED, 0x00), // Yellow
            SkColorSetRGB(0x00, 0x80, 0x26), // Green
            SkColorSetRGB(0x00, 0x4D, 0xFF), // Blue
            SkColorSetRGB(0x75, 0x07, 0x87)  // Violet
        };
        SkPoint points[] = {
            {0, 0}, {0, 100},
        };
        auto shader = SkGradientShader::MakeLinear(
            points, colors, nullptr, std::size(colors), SkTileMode::kClamp);
        paint.setShader(shader);
        SkRRect rrect = SkRRect::MakeRectXY({40, 20, 100, 50}, 5, 5);
        canvas->drawRRect(rrect, paint);
    }

    SkPixmap pixmap;
    if (surface->peekPixels(&pixmap)) {
        if (!SkPngEncoder::Encode(&output, pixmap, {})) {
          printf("Cannot write output\n");
          return 1;
        }
    } else {
      printf("Cannot readback on surface\n");
      return 1;
    }
    return 0;
}
