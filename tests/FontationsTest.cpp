/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/ports/SkTypeface_fontations.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>
#include <vector>

namespace {
const char kFontResource[] = "fonts/ahem.ttf";
const char kTtcResource[] = "fonts/test.ttc";
const char kNoCapHeightResource[] = "fonts/DejaVuSans.subset.ttf";
const char kNoCapHeightNoHxResource[] = "fonts/DejaVuSans.subset_noHx.ttf";
const char kSbixResource[] = "fonts/sbix.ttf";
const char kHintGaspResource[] = "fonts/hintgasp.ttf";
const char kVariableResource[] = "fonts/test_glyphs-glyf_colr_1_variable.ttf";
constexpr size_t kNumVariableAxes = 44;

struct AxisExpectation {
    SkFourByteTag tag;
    float minValue;
    float defValue;
    float maxValue;
} axisExpectations[] = {
        {SkSetFourByteTag('S', 'W', 'P', 'S'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'W', 'P', 'E'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'W', 'C', '1'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '2'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '3'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'W', 'C', '4'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('S', 'C', 'O', 'X'), -200., 0.0, 200.},
        {SkSetFourByteTag('S', 'C', 'O', 'Y'), -200., 0.0, 200.},
        {SkSetFourByteTag('S', 'C', 'S', 'X'), -2.0, 0.0, 1.9999389648437},
        {SkSetFourByteTag('S', 'C', 'S', 'Y'), -2.0, 0.0, 1.9999389648437},
        {SkSetFourByteTag('G', 'R', 'X', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'X', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'X', '2'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'Y', '2'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'R', '0'), -1000, 0.0, 1000},
        {SkSetFourByteTag('G', 'R', 'R', '1'), -1000, 0.0, 1000},
        {SkSetFourByteTag('C', 'O', 'L', '1'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('C', 'O', 'L', '2'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('C', 'O', 'L', '3'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('R', 'O', 'T', 'A'), 0.0, 0.0, 539.989013671875},
        {SkSetFourByteTag('R', 'O', 'T', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('R', 'O', 'T', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('S', 'K', 'X', 'A'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'K', 'Y', 'A'), -90.0, 0.0, 90.0},
        {SkSetFourByteTag('S', 'K', 'C', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('S', 'K', 'C', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'R', 'X', 'X'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'Y', 'X'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'X', 'Y'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'Y', 'Y'), -2.0, 0.0, 2.0},
        {SkSetFourByteTag('T', 'R', 'D', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'R', 'D', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'L', 'D', 'X'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('T', 'L', 'D', 'Y'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'X', 'I'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'Y', 'I'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'X', 'A'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'Y', 'A'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('C', 'L', 'I', 'O'), -500.0, 0.0, 500.0},
        {SkSetFourByteTag('A', 'P', 'H', '1'), -1.0, 0.0, 0.0},
        {SkSetFourByteTag('A', 'P', 'H', '2'), -1.0, 0.0, 0.0},
        {SkSetFourByteTag('A', 'P', 'H', '3'), -1.0, 0.0, 0.0},
};
}  // namespace

DEF_TEST(Fontations_DoNotMakeFromNull, reporter) {
    std::unique_ptr<SkStreamAsset> nullStream = SkMemoryStream::MakeDirect(nullptr, 0);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(nullStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromNonSfnt, reporter) {
    char notAnSfnt[] = "I_AM_NOT_AN_SFNT";
    std::unique_ptr<SkStreamAsset> notSfntStream =
            SkMemoryStream::MakeDirect(notAnSfnt, std::size(notAnSfnt));
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(std::move(notSfntStream), SkFontArguments()));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_MakeFromFont, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollection, reporter) {
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), SkFontArguments()));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_MakeFromCollectionNonNullIndex, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, probeTypeface);
}

DEF_TEST(Fontations_DoNotMakeFromCollection_Invalid_Index, reporter) {
    SkFontArguments args;
    args.setCollectionIndex(1000);
    sk_sp<SkTypeface> probeTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kTtcResource), args));
    REPORTER_ASSERT(reporter, !probeTypeface);
}

DEF_TEST(Fontations_TableData, reporter) {
    constexpr size_t kNameTableSize = 11310;
    constexpr size_t kTestOffset = 1310;
    constexpr size_t kTestLength = 500;
    char destBuffer[kNameTableSize] = {0};
    sk_sp<SkTypeface> testTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    SkFourByteTag nameTableTag = SkSetFourByteTag('n', 'a', 'm', 'e');
    SkFourByteTag nonExistantTag = SkSetFourByteTag('0', 'X', '0', 'X');

    // Getting size without buffer.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize, nullptr) ==
                            kNameTableSize);
    // Reading full table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize, destBuffer) ==
                            kNameTableSize);
    // Reading restricted length.
    REPORTER_ASSERT(
            reporter,
            testTypeface->getTableData(nameTableTag, 0, kTestLength, destBuffer) == kTestLength);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kTestOffset, kTestLength, destBuffer) == kTestLength);
    // Reading at an offset.
    REPORTER_ASSERT(
            reporter,
            testTypeface->getTableData(nameTableTag, kTestOffset, kNameTableSize, destBuffer) ==
                    kNameTableSize - kTestOffset);

    // Reading from offset past table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kNameTableSize, kNameTableSize, destBuffer) == 0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, kNameTableSize, 0, nullptr) == 0);
    // Reading one byte before end of table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(
                            nameTableTag, kNameTableSize - 1, kNameTableSize, destBuffer) == 1);
    // Trying to start reading at an offset past table start.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kNameTableSize + 10, destBuffer) ==
                            kNameTableSize);
    // Restricting length without target buffer.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nameTableTag, 0, kTestLength, nullptr) ==
                            kTestLength);

    // Trying to access non-existant table.
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, 0, kNameTableSize, destBuffer) ==
                            0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, 0, 0, nullptr) ==
                            0);
    REPORTER_ASSERT(reporter,
                    testTypeface->getTableData(nonExistantTag, kTestOffset, 0, nullptr) == 0);
}

DEF_TEST(Fontations_TableTags, reporter) {
    constexpr size_t kNumTags = 11;
    SkFourByteTag tagsBuffer[kNumTags] = {0};
    sk_sp<SkTypeface> testTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kFontResource), SkFontArguments()));
    SkFourByteTag firstTag = SkSetFourByteTag('O', 'S', '/', '2');
    SkFourByteTag lastTag = SkSetFourByteTag('p', 'o', 's', 't');

    // Keep these in the old ptr style (not readTableTags) until we're sure
    // we've tested that adaptor through chrome (e.g. on 32bit machines)
    REPORTER_ASSERT(reporter, testTypeface->countTables() == kNumTags);

    REPORTER_ASSERT(reporter, testTypeface->readTableTags(tagsBuffer) == kNumTags);
    REPORTER_ASSERT(reporter, tagsBuffer[0] == firstTag);
    REPORTER_ASSERT(reporter, tagsBuffer[kNumTags - 1] == lastTag);
}

DEF_TEST(Fontations_VariationPosition, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    // Everything at default.
    const int numAxes = variableTypeface->getVariationDesignPosition({});
    REPORTER_ASSERT(reporter, numAxes == kNumVariableAxes, "numAxes: %d", numAxes);

    SkFontArguments::VariationPosition::Coordinate kSwpsCoordinateFirst = {SkSetFourByteTag('S', 'W', 'P', 'S'), 25};
    SkFontArguments::VariationPosition::Coordinate kSwpsCoordinateSecond = {SkSetFourByteTag('S', 'W', 'P', 'S'), 55};
    SkFontArguments::VariationPosition::Coordinate kSwpeCoordinate = {SkSetFourByteTag('S', 'W', 'P', 'E'), 45};
    SkFontArguments::VariationPosition::Coordinate kInvalidCoordinate = {SkSetFourByteTag('_', '_', '_', '_'), 0};

    // 'SWPS' and 'SWPE' exist. Second 'SWPS' should override first, invalid tag should be stripped.
    SkFontArguments::VariationPosition::Coordinate cloneCoordinates[4] = {
            kSwpsCoordinateFirst, kSwpsCoordinateSecond, kSwpeCoordinate, kInvalidCoordinate};

    SkFontArguments::VariationPosition clonePosition;
    clonePosition.coordinates = cloneCoordinates;
    clonePosition.coordinateCount = 4;

    sk_sp<SkTypeface> cloneTypeface = variableTypeface->makeClone(
            SkFontArguments().setVariationDesignPosition(clonePosition));
    const int cloneNumAxes = cloneTypeface->getVariationDesignPosition({});
    REPORTER_ASSERT(reporter, cloneNumAxes == kNumVariableAxes, "clonedNumAxes: %d", cloneNumAxes);

    SkFontArguments::VariationPosition::Coordinate retrieveCoordinates[kNumVariableAxes] = {};

    // Error when providing too little space.
    const int badClonedNumAxes = cloneTypeface->getVariationDesignPosition({retrieveCoordinates,1});
    REPORTER_ASSERT(reporter, badClonedNumAxes == -1, "badClonedNumAxes: %d", badClonedNumAxes);

    const int retrievedClonedNumAxes =
            cloneTypeface->getVariationDesignPosition(retrieveCoordinates);
    REPORTER_ASSERT(reporter, retrievedClonedNumAxes == kNumVariableAxes,
                    "retrievedClonedNumAxes: %d", retrievedClonedNumAxes);
    REPORTER_ASSERT(reporter,
                    retrieveCoordinates[0].axis  == kSwpsCoordinateSecond.axis &&
                    retrieveCoordinates[0].value == kSwpsCoordinateSecond.value);
    REPORTER_ASSERT(reporter,
                    retrieveCoordinates[1].axis  == kSwpeCoordinate.axis &&
                    retrieveCoordinates[1].value == kSwpeCoordinate.value);
}

DEF_TEST(Fontations_VariationParameters, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters({}) == kNumVariableAxes);

    SkFontParameters::Variation::Axis axes[kNumVariableAxes] = {};
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(axes) == kNumVariableAxes);

    for (size_t i = 0; i < kNumVariableAxes; ++i) {
        REPORTER_ASSERT(reporter, axes[i].tag == axisExpectations[i].tag);
        REPORTER_ASSERT(reporter, axes[i].min == axisExpectations[i].minValue);
        REPORTER_ASSERT(reporter, axes[i].def == axisExpectations[i].defValue);
        REPORTER_ASSERT(reporter, axes[i].max == axisExpectations[i].maxValue);
    }
}

DEF_TEST(Fontations_VariationParameters_BufferTooSmall, reporter) {
    sk_sp<SkTypeface> variableTypeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kVariableResource), SkFontArguments()));
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters({}) == kNumVariableAxes);

    constexpr size_t kArrayTooSmall = 3;
    SkFontParameters::Variation::Axis axes[kArrayTooSmall] = {};
    REPORTER_ASSERT(reporter,
                    variableTypeface->getVariationDesignParameters(axes) == -1);
}

DEF_TEST(Fontations_SyntheticCapHeight, reporter) {
    sk_sp<SkTypeface> noCapHeightTypeface(SkTypeface_Make_Fontations(
            GetResourceAsStream(kNoCapHeightResource), SkFontArguments()));
    sk_sp<SkTypeface> noCapHeightNoHxTypeface(SkTypeface_Make_Fontations(
            GetResourceAsStream(kNoCapHeightNoHxResource), SkFontArguments()));
    SkASSERT_RELEASE(noCapHeightTypeface);
    SkASSERT_RELEASE(noCapHeightNoHxTypeface);

    SkFont capHeightFont(noCapHeightTypeface);
    SkFont capHeightFontNoHx(noCapHeightNoHxTypeface);

    capHeightFont.setSize(12);
    capHeightFontNoHx.setSize(12);

    SkFontMetrics metrics;

    capHeightFont.getMetrics(&metrics);
    const SkScalar kHCharHeight = 9.0;
    REPORTER_ASSERT(reporter, metrics.fCapHeight == kHCharHeight);

    capHeightFontNoHx.getMetrics(&metrics);
    SkGlyphID glyphId = noCapHeightNoHxTypeface->unicharToGlyph('H');
    REPORTER_ASSERT(reporter, glyphId == 0, "Glyph lookup for H should fail, but was: %u", glyphId);

    const SkScalar kExpected = 11.138672;
    REPORTER_ASSERT(reporter, metrics.fCapHeight == kExpected, "Metrics mismatch: %f vs. %f", kExpected, metrics.fCapHeight);
}

DEF_TEST(Fontations_SyntheticXHeight, reporter) {
    sk_sp<SkTypeface> noXHeightTypeface(SkTypeface_Make_Fontations(
            GetResourceAsStream(kNoCapHeightResource), SkFontArguments()));
    sk_sp<SkTypeface> noXHeightNoHxTypeface(SkTypeface_Make_Fontations(
            GetResourceAsStream(kNoCapHeightNoHxResource), SkFontArguments()));
    SkASSERT_RELEASE(noXHeightTypeface);
    SkASSERT_RELEASE(noXHeightNoHxTypeface);

    SkFont xHeightFont(noXHeightTypeface);
    SkFont xHeightFontNoHx(noXHeightNoHxTypeface);

    xHeightFont.setSize(12);
    xHeightFontNoHx.setSize(12);

    SkFontMetrics metrics;

    xHeightFont.getMetrics(&metrics);
    const SkScalar kXCharHeight = 7.0;
    REPORTER_ASSERT(reporter,
                    metrics.fXHeight == kXCharHeight,
                    "Expected: %f vs actual: %f\n",
                    kXCharHeight,
                    metrics.fXHeight);

    xHeightFontNoHx.getMetrics(&metrics);
    SkGlyphID glyphId = noXHeightNoHxTypeface->unicharToGlyph('x');
    REPORTER_ASSERT(reporter, glyphId == 0, "Glyph lookup for x should fail, but was: %u", glyphId);

    // xHeight falls back to ascent as well.
    const SkScalar kExpected = 11.138672;
    REPORTER_ASSERT(reporter, metrics.fXHeight == kExpected, "Metrics mismatch: %f vs. %f", kExpected, metrics.fXHeight);
}

DEF_TEST(Fontations_EmbeddedBitmapDisabledFallsBackToOutline, reporter) {
    // sbix.ttf carries both an `sbix` embedded bitmap table and a `glyf` outline
    // table. When the client disables embedded bitmaps - for example Chromium
    // honoring fontconfig's embeddedbitmap setting or Blink's bitmap glyph block
    // list, both expressed through SkFont::setEmbeddedBitmaps(false) - the
    // outline must be used instead of the embedded bitmap. The Fontations scaler
    // context used to ignore that request and always rasterize the embedded
    // bitmap, mismatching the FreeType backend (FT_LOAD_NO_BITMAP). Regression
    // test for https://issues.chromium.org/issues/526181509.
    sk_sp<SkTypeface> typeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kSbixResource), SkFontArguments()));
    if (!typeface) {
        return;  // Resource not available in this build configuration.
    }

    auto maskFormatForGlyph = [&](SkGlyphID glyphId, bool embeddedBitmaps) {
        SkFont font(typeface, 16.0f);
        font.setEmbeddedBitmaps(embeddedBitmaps);
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                font, SkPaint(), SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());
        SkBulkGlyphMetricsAndImages glyphs(strikeSpec);
        return glyphs.glyph(SkPackedGlyphID(glyphId))->maskFormat();
    };

    // Find a glyph that rasterizes from an embedded (color) bitmap when embedded
    // bitmaps are enabled.
    const int glyphCount = typeface->countGlyphs();
    SkGlyphID bitmapGlyphId = 0;
    for (int glyphId = 1; glyphId < glyphCount; ++glyphId) {
        if (maskFormatForGlyph(static_cast<SkGlyphID>(glyphId), /*embeddedBitmaps=*/true) ==
            SkMask::kARGB32_Format) {
            bitmapGlyphId = static_cast<SkGlyphID>(glyphId);
            break;
        }
    }
    REPORTER_ASSERT(reporter, bitmapGlyphId != 0,
                    "Expected sbix.ttf to expose an embedded bitmap glyph.");
    if (bitmapGlyphId == 0) {
        return;
    }

    // With embedded bitmaps disabled the same glyph must fall back to the
    // outline, i.e. it is no longer rasterized from the ARGB32 embedded bitmap.
    REPORTER_ASSERT(reporter,
                    maskFormatForGlyph(bitmapGlyphId, /*embeddedBitmaps=*/false) !=
                            SkMask::kARGB32_Format,
                    "Embedded bitmap used for glyph %u despite being disabled.", bitmapGlyphId);
}

namespace {
struct GlyphRendering {
    SkMask::Format format = SkMask::kA8_Format;
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    std::vector<uint8_t> image;
    bool operator==(const GlyphRendering& o) const {
        return format == o.format && left == o.left && top == o.top && width == o.width &&
               height == o.height && image == o.image;
    }
};

GlyphRendering renderGlyph(const sk_sp<SkTypeface>& typeface,
                           SkGlyphID glyphId,
                           float textSize,
                           bool embeddedBitmaps) {
    SkFont font(typeface, textSize);
    font.setEmbeddedBitmaps(embeddedBitmaps);
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, SkPaint(), SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());
    SkBulkGlyphMetricsAndImages glyphs(strikeSpec);
    const SkGlyph* glyph = glyphs.glyph(SkPackedGlyphID(glyphId));

    GlyphRendering rendering;
    rendering.format = glyph->maskFormat();
    rendering.left = glyph->left();
    rendering.top = glyph->top();
    rendering.width = glyph->width();
    rendering.height = glyph->height();
    if (glyph->image() && glyph->height() > 0) {
        const uint8_t* bytes = static_cast<const uint8_t*>(glyph->image());
        rendering.image.assign(bytes, bytes + glyph->rowBytes() * glyph->height());
    }
    return rendering;
}
}  // namespace

DEF_TEST(Fontations_FarEmbeddedBitmapStrikePrefersOutline, reporter) {
    // hintgasp.ttf has glyf outlines and monochrome EBDT/EBLC strikes at ppem 9,
    // 11, 13 and 15 for glyphs 0-3. Near a strike the embedded bitmap is used;
    // far from any strike the outline must be used rather than an upscaled
    // bitmap. Regression test for https://issues.chromium.org/issues/526181509.
    sk_sp<SkTypeface> typeface(
            SkTypeface_Make_Fontations(GetResourceAsStream(kHintGaspResource), SkFontArguments()));
    if (!typeface) {
        return;
    }
    const int glyphCount = typeface->countGlyphs();

    // Sanity check: at a size matching an embedded strike, toggling embedded
    // bitmaps changes the rendering (the bitmap is actually used).
    bool embeddedBitmapUsed = false;
    for (float strikeSize : {9.f, 11.f, 13.f, 15.f}) {
        for (int glyphId = 1; glyphId < glyphCount; ++glyphId) {
            const SkGlyphID id = static_cast<SkGlyphID>(glyphId);
            if (!(renderGlyph(typeface, id, strikeSize, /*embeddedBitmaps=*/true) ==
                  renderGlyph(typeface, id, strikeSize, /*embeddedBitmaps=*/false))) {
                embeddedBitmapUsed = true;
                break;
            }
        }
        if (embeddedBitmapUsed) {
            break;
        }
    }
    REPORTER_ASSERT(reporter, embeddedBitmapUsed,
                    "Expected hintgasp.ttf to use an embedded bitmap at a strike size.");

    // At a size far from any available strike, enabling embedded bitmaps must not
    // change the rendering: the outline is used rather than an upscaled embedded
    // bitmap.
    constexpr float kFarFromStrikeSize = 200.f;
    for (int glyphId = 1; glyphId < glyphCount; ++glyphId) {
        const SkGlyphID id = static_cast<SkGlyphID>(glyphId);
        REPORTER_ASSERT(reporter,
                        renderGlyph(typeface, id, kFarFromStrikeSize, /*embeddedBitmaps=*/true) ==
                                renderGlyph(typeface, id, kFarFromStrikeSize,
                                            /*embeddedBitmaps=*/false),
                        "Glyph %u rendering changed when toggling embedded bitmaps at size %f; an "
                        "embedded bitmap strike was scaled instead of using the outline.",
                        id, kFarFromStrikeSize);
    }
}
