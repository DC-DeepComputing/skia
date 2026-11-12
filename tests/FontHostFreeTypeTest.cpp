/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeSpec.h"
#include "src/ports/SkFontHost_FreeType_common.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <ft2build.h>

#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

namespace {

sk_sp<SkTypeface> make_fixed_bounds_typeface(SkRect bounds, SkMask::Format format) {
    class FixedBoundsTypeface final : public TestEmptyTypeface {
    public:
        FixedBoundsTypeface(SkRect bounds, SkMask::Format format)
                : fBounds(bounds), fFormat(format) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(FixedBoundsTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
                        : SkScalerContext(tf, effects, desc), fTypeface(tf) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override {
                    GlyphMetrics mx(fTypeface.fFormat);
                    mx.neverRequestPath = true;
                    mx.bounds = fTypeface.fBounds;
                    return mx;
                }

                void generateImage(const SkGlyph&, void*) override {}

                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }

                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const FixedBoundsTypeface& fTypeface;
            };

            return std::make_unique<ScalerContext>(
                    *const_cast<FixedBoundsTypeface*>(this), effects, desc);
        }

    private:
        const SkRect fBounds;
        const SkMask::Format fFormat;
    };

    return sk_make_sp<FixedBoundsTypeface>(bounds, format);
}

SkGlyph make_test_glyph(SkRect bounds,
                        SkMask::Format format,
                        SkPixelGeometry pixelGeometry,
                        SkPackedGlyphID packedID,
                        SkArenaAlloc* alloc) {
    sk_sp<SkTypeface> tf = make_fixed_bounds_typeface(bounds, format);
    SkFont font(tf, 10.0f);
    if (format == SkMask::kLCD16_Format) {
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    } else {
        font.setEdging(SkFont::Edging::kAntiAlias);
    }
    font.setSubpixel(true);
    SkPaint paint;
    SkStrikeSpec spec = SkStrikeSpec::MakeMask(font,
                                               paint,
                                               SkSurfaceProps(0, pixelGeometry),
                                               SkScalerContextFlags::kNone,
                                               SkMatrix::I());
    std::unique_ptr<SkScalerContext> ctx = spec.createScalerContext();
    return ctx->makeGlyph(packedID, alloc);
}

struct GuardedBuffer {
    static constexpr size_t kCanaryBytes = 64;
    static constexpr uint8_t kCanary = 0x5A;

    std::vector<uint8_t> storage;
    size_t payloadSize = 0;

    explicit GuardedBuffer(size_t size)
            : storage(size + 2 * kCanaryBytes, kCanary), payloadSize(size) {}

    uint8_t* data() { return storage.data() + kCanaryBytes; }

    bool canariesIntact() const {
        for (size_t i = 0; i < kCanaryBytes; ++i) {
            if (storage[i] != kCanary || storage[kCanaryBytes + payloadSize + i] != kCanary) {
                return false;
            }
        }
        return true;
    }

    bool hasNonZeroPayload() const {
        return std::any_of(storage.begin() + kCanaryBytes,
                           storage.begin() + kCanaryBytes + payloadSize,
                           [](uint8_t b) { return b != 0; });
    }
};

}  // namespace

// Directly exercises SkScalerContextFTUtils::generateGlyphImage when a loaded FreeType outline
// diverges from SkGlyph's pre-computed bounds (e.g. horizontal/vertical LCD16 subpixel clipping and
// SkFDot6 subpixel translation overflow).
DEF_TEST(FreeType_GenerateGlyphImageDivergentOutline, reporter) {
    sk_sp<SkData> fontData = GetResourceAsData("fonts/ahem.ttf");
    if (!fontData) {
        return;
    }

    FT_Library rawLib = nullptr;
    if (FT_Init_FreeType(&rawLib) != 0) {
        ERRORF(reporter, "FT_Init_FreeType failed");
        return;
    }
    std::unique_ptr<FT_LibraryRec_, decltype(&FT_Done_FreeType)> library(rawLib, FT_Done_FreeType);

    FT_Face rawFace = nullptr;
    if (FT_New_Memory_Face(
                library.get(), fontData->bytes(), SkTo<FT_Long>(fontData->size()), 0, &rawFace) !=
        0) {
        ERRORF(reporter, "FT_New_Memory_Face failed");
        return;
    }
    std::unique_ptr<FT_FaceRec_, decltype(&FT_Done_Face)> face(rawFace, FT_Done_Face);

    REPORTER_ASSERT(reporter, FT_Set_Pixel_Sizes(face.get(), 0, 10) == 0);
    REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
    REPORTER_ASSERT(reporter, face->glyph->format == FT_GLYPH_FORMAT_OUTLINE);

    FT_BBox initBox;
    FT_Outline_Get_CBox(&face->glyph->outline, &initBox);
    const SkRect baseBounds = SkRect::MakeLTRB(SkIntToScalar(initBox.xMin >> 6),
                                               SkIntToScalar(-(initBox.yMax >> 6)),
                                               SkIntToScalar(initBox.xMax >> 6),
                                               SkIntToScalar(-(initBox.yMin >> 6)));
    REPORTER_ASSERT(reporter, baseBounds.width() == 10 && baseBounds.height() == 10);

    // 1. Horizontal and vertical kLCD16_Format outline shifts: exercises pixel-vs-3x-subpixel
    // intersection and 3x buffer/dimension alignment in copyFT2LCD16.
    struct ShiftCase {
        int dxPx;
        int dyPx;
        bool expectNonZero;
    };
    // +-4 px shifts are intentionally not multiples of 3, exercising 3x subpixel stride scaling;
    // +-15 px shifts move the 10x10 pixel glyph completely outside baseBounds while remaining
    // inside the [0..30] false-overlap window that unscaled 3x subpixel dimensions would hit.
    constexpr ShiftCase kShifts[] = {
            {0, 0, true},
            {-4, 0, true},
            {4, 0, true},
            {0, -4, true},
            {0, 4, true},
            {-15, 0, false},
            {15, 0, false},
            {0, -15, false},
            {0, 15, false},
    };

    for (bool doVert : {false, true}) {
        const SkPixelGeometry geom = doVert ? kRGB_V_SkPixelGeometry : kRGB_H_SkPixelGeometry;
        SkArenaAlloc alloc{256};
        SkGlyph glyph = make_test_glyph(
                baseBounds, SkMask::kLCD16_Format, geom, SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kLCD16_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() == glyph.rowBytes() * glyph.height());

        SkScalerContextFTUtils utils{};
        const uint32_t flags = doVert ? SkScalerContext::kLCD_Vertical_Flag : 0;
        utils.init(SK_ColorBLACK, static_cast<SkScalerContext::Flags>(flags));
        SkMaskGamma::PreBlend noPreBlend;

        for (const ShiftCase& tc : kShifts) {
            REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
            FT_Outline_Translate(&face->glyph->outline, tc.dxPx * 64, tc.dyPx * 64);

            GuardedBuffer buf(glyph.imageSize());
            utils.generateGlyphImage(face.get(), glyph, buf.data(), SkMatrix::I(), noPreBlend);

            REPORTER_ASSERT(reporter, buf.canariesIntact());
            REPORTER_ASSERT(reporter, buf.hasNonZeroPayload() == tc.expectNonZero);
        }
    }

    // 2. Non-LCD subpixel translation overflow: bbox.xMin / bbox.yMin individually fit in SkFDot6
    // (int32_t), while adding subpixel dx (+32) or dy (-32) would overflow/underflow SkFDot6.
    {
        SkArenaAlloc alloc{256};
        SkGlyph subpixelGlyph =
                make_test_glyph(baseBounds,
                                SkMask::kA8_Format,
                                kUnknown_SkPixelGeometry,
                                SkPackedGlyphID((SkGlyphID)1, SK_FixedHalf, SK_FixedHalf),
                                &alloc);
        REPORTER_ASSERT(reporter, subpixelGlyph.maskFormat() == SkMask::kA8_Format);

        SkScalerContextFTUtils utils{};
        utils.init(SK_ColorBLACK, SkScalerContext::kSubpixelPositioning_Flag);
        SkMaskGamma::PreBlend noPreBlend;

        // xMin + dx (+32) > SK_MaxS32
        REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
        for (int i = 0; i < face->glyph->outline.n_points; ++i) {
            face->glyph->outline.points[i].x = SK_MaxS32 - 16;
        }
        GuardedBuffer xOverflowBuf(subpixelGlyph.imageSize());
        utils.generateGlyphImage(
                face.get(), subpixelGlyph, xOverflowBuf.data(), SkMatrix::I(), noPreBlend);
        REPORTER_ASSERT(reporter, xOverflowBuf.canariesIntact());
        REPORTER_ASSERT(reporter, !xOverflowBuf.hasNonZeroPayload());

        // yMin + dy (-32) < SK_MinS32
        REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
        for (int i = 0; i < face->glyph->outline.n_points; ++i) {
            face->glyph->outline.points[i].y = SK_MinS32 + 16;
        }
        GuardedBuffer yUnderflowBuf(subpixelGlyph.imageSize());
        utils.generateGlyphImage(
                face.get(), subpixelGlyph, yUnderflowBuf.data(), SkMatrix::I(), noPreBlend);
        REPORTER_ASSERT(reporter, yUnderflowBuf.canariesIntact());
        REPORTER_ASSERT(reporter, !yUnderflowBuf.hasNonZeroPayload());

        // xMin + dx (+32) fits in SkFDot6, while xShift (dx - SK_MinS32) > SK_MaxS32.
        REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
        for (int i = 0; i < face->glyph->outline.n_points; ++i) {
            face->glyph->outline.points[i].x = SK_MinS32;
        }
        GuardedBuffer xShiftOverflowBuf(subpixelGlyph.imageSize());
        utils.generateGlyphImage(
                face.get(), subpixelGlyph, xShiftOverflowBuf.data(), SkMatrix::I(), noPreBlend);
        REPORTER_ASSERT(reporter, xShiftOverflowBuf.canariesIntact());
        REPORTER_ASSERT(reporter, !xShiftOverflowBuf.hasNonZeroPayload());

        // xMin and xMax both fit in SkFDot6, while xMax + xShift > SK_MaxS32.
        REPORTER_ASSERT(reporter, FT_Load_Char(face.get(), 'X', FT_LOAD_NO_BITMAP) == 0);
        if (face->glyph->outline.n_points >= 2) {
            face->glyph->outline.points[0].x = -1500000000;
            for (int i = 1; i < face->glyph->outline.n_points; ++i) {
                face->glyph->outline.points[i].x = 1500000000;
            }
            GuardedBuffer spanOverflowBuf(subpixelGlyph.imageSize());
            utils.generateGlyphImage(
                    face.get(), subpixelGlyph, spanOverflowBuf.data(), SkMatrix::I(), noPreBlend);
            REPORTER_ASSERT(reporter, spanOverflowBuf.canariesIntact());
            REPORTER_ASSERT(reporter, !spanOverflowBuf.hasNonZeroPayload());
        }
    }
}
