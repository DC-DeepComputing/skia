/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Kept out of SkParagraphTest.cpp: ApplicationServices declares a global TextRange that clashes
// with skia::textlayout::TextRange there
#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_MAC) && defined(SK_TYPEFACE_FACTORY_CORETEXT)

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_mac.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "src/utils/mac/SkUniqueCFRef.h"
#include "tests/Test.h"

#include <algorithm>
#include <iterator>

using namespace skia::textlayout;

// A face with an 'opsz' axis is shaped at opsz == font size unless the style sets 'opsz' (CSS
// font-optical-sizing: auto). Expected widths come from the same face with an explicit 'opsz'
DEF_TEST(SkParagraph_AutoOpticalSizeFollowsFontSize, reporter) {
    constexpr SkFourByteTag kOpsz = SkSetFourByteTag('o', 'p', 's', 'z');
    constexpr SkFourByteTag kWght = SkSetFourByteTag('w', 'g', 'h', 't');
    constexpr SkScalar kBaseSize = 17;
    // Below the .SFNS default opsz of 28, above which its advances no longer change, so a clone
    // at the axis default fails as well
    constexpr SkScalar kFontSize = 24;

    // The typeface keeps the opsz of the CTFont it was made from, here 17
    SkUniqueCFRef<CTFontRef> ctFont(
            CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, kBaseSize, nullptr));
    REPORTER_ASSERT(reporter, ctFont);
    if (!ctFont) {
        return;
    }
    sk_sp<SkTypeface> system = SkMakeTypefaceFromCTFont(ctFont.get());
    SkFontParameters::Variation::Axis axes[16];
    int axisCount = system->getVariationDesignParameters(axes);
    // The system font is variable with an 'opsz' axis only since macOS 10.15
    if (axisCount <= 0 || axisCount > (int)std::size(axes) ||
        std::none_of(axes, axes + axisCount, [&](const auto& axis) {
            return axis.tag == kOpsz && axis.min < kFontSize && kFontSize <= axis.max;
        })) {
        return;
    }

    auto fontProvider = sk_make_sp<TypefaceFontProvider>();
    fontProvider->registerTypeface(system, SkString("SystemUI"));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(fontProvider);
    fontCollection->disableFontFallback();
    auto unicode = sk_ref_sp(SkShapers::BestAvailable()->getUnicode());

    using Coordinate = SkFontArguments::VariationPosition::Coordinate;
    auto width = [&](SkSpan<const Coordinate> coordinates) {
        TextStyle style;
        style.setFontFamilies({SkString("SystemUI")});
        style.setFontSize(kFontSize);
        if (!coordinates.empty()) {
            SkFontArguments args;
            args.setVariationDesignPosition({coordinates.data(), (int)coordinates.size()});
            style.setFontArguments(args);
        }
        ParagraphBuilderImpl builder(ParagraphStyle(), fontCollection, unicode);
        builder.pushStyle(style);
        builder.addText("Hamburgefonstiv");
        auto paragraph = builder.Build();
        paragraph->layout(SK_ScalarInfinity);
        return paragraph->getMaxIntrinsicWidth();
    };

    const Coordinate opszBase[] = {{kOpsz, kBaseSize}};
    const Coordinate opszSize[] = {{kOpsz, kFontSize}};
    const Coordinate wght[] = {{kWght, 700}};
    const Coordinate wghtOpszSize[] = {{kWght, 700}, {kOpsz, kFontSize}};
    SkScalar atBase = width(opszBase);
    SkScalar atSize = width(opszSize);
    SkScalar automatic = width({});
    REPORTER_ASSERT(reporter, !SkScalarNearlyEqual(atBase, atSize), "%f %f", atBase, atSize);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(automatic, atSize),
                    "auto %f explicit %f",
                    automatic,
                    atSize);
    // Axes other than 'opsz' in the style do not turn auto optical sizing off
    SkScalar wghtOnly = width(wght);
    SkScalar wghtAtSize = width(wghtOpszSize);
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(wghtOnly, wghtAtSize),
                    "wght %f wght+opsz %f",
                    wghtOnly,
                    wghtAtSize);
}

#endif
