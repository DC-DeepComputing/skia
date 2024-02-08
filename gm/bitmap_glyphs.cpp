/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
#include "include/ports/SkTypeface_fontations.h"
#endif

#include <string.h>

namespace skiagm {

namespace {
// const char kSbixTestFontPath[] = "fonts/apple_color_emoji_subset.ttf";
const char kCbdtTestFontPath[] = "fonts/cbdt.ttf";
const uint32_t emoji_test_string[] = {0x1F3BA, 0x1F9FA};
const uint32_t cbdt_test_string[] = {0x2662};
const SkScalar kFontSize = 200;

enum TypefaceBackend { UseDefault, UseFontations };

// TODO(b/318667611): Move the explicit instantation to font manager for Fontations.
#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
constexpr auto kBackend = TypefaceBackend::UseFontations;
#else
constexpr auto kBackend = TypefaceBackend::UseDefault;
#endif

template <TypefaceBackend variant>
sk_sp<SkTypeface> MakeTypefaceFromResource(const char* resource, const SkFontArguments& args);

#if defined(SK_TYPEFACE_FACTORY_FONTATIONS)
template <>
sk_sp<SkTypeface> MakeTypefaceFromResource<UseFontations>(const char* resource,
                                                          const SkFontArguments& args) {
    std::unique_ptr<SkStreamAsset> resourceStream(GetResourceAsStream(resource, false));
    return SkTypeface_Make_Fontations(std::move(resourceStream), args);
}
#else
template <>
sk_sp<SkTypeface> MakeTypefaceFromResource<UseDefault>(const char* resource,
                                                       const SkFontArguments& args) {
    return ToolUtils::TestFontMgr()->makeFromStream(GetResourceAsStream(resource), args);
}
#endif

}  // namespace

class BitmapGlyphsGM : public GM {
public:
    BitmapGlyphsGM() = default;

protected:
    sk_sp<SkTypeface> fTypefaceDefault;

    void onOnceBeforeDraw() override {
        // fTypefaceDefault = MakeTypefaceFromResource<kBackend>(kSbixTestFontPath, SkFontArguments());
        fTypefaceDefault = MakeTypefaceFromResource<kBackend>(kCbdtTestFontPath, SkFontArguments());
    }

    SkString getName() const override { return SkString("bitmap_glyphs"); }

    SkISize getISize() override { return SkISize::Make(1000, 400); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;

        canvas->translate(10, 20);

        if (!fTypefaceDefault) {
            *errorMsg = "Did not recognize test font format.";
            return DrawResult::kSkip;
        }

        SkFontMetrics metrics;
        SkScalar y = 0;

        SkFont defaultFont(fTypefaceDefault);
        defaultFont.setSize(kFontSize);

        defaultFont.getMetrics(&metrics);
        y += -metrics.fAscent;
        // Set a recognizable foreground color which is not to be overriden.
        paint.setColor(SK_ColorGRAY);
        // Draw the default palette on the left, for COLRv0 and COLRv1.
        canvas->drawSimpleText(emoji_test_string,
//                               std::size(emoji_test_string) * sizeof(uint32_t),
                               std::size(cbdt_test_string) * sizeof(uint32_t),
                               SkTextEncoding::kUTF32,
                               0,
                               y,
                               defaultFont,
                               paint);

        y += metrics.fDescent + metrics.fLeading;

        return DrawResult::kOk;
    }

private:
    using INHERITED = GM;
};

DEF_GM(return new BitmapGlyphsGM());

}  // namespace skiagm
