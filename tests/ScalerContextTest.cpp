/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkScalerContext.h"
#include "tests/Test.h"

#include <array>
#include <cstdint>
#include <initializer_list>

namespace {
class PathMaskScalerContext : public SkScalerContext {
public:
    using SkScalerContext::GenerateImageFromPath;
};

struct RestoreTextRenderingParams {
    SkGraphics::TextRenderingParams saved = SkGraphics::GetTextRenderingParams();
    ~RestoreTextRenderingParams() { SkGraphics::SetTextRenderingParams(saved); }
};
}  // namespace

DEF_SERIAL_TEST(ScalerContext_TextRenderingParams, reporter) {
    RestoreTextRenderingParams restore;
    using A8 = SkGraphics::A8GammaCorrection;
    using Filter = SkGraphics::PathLCDFilter;
    for (auto mode : {A8::kDefault, A8::kDisabled, A8::kEnabled}) {
        SkGraphics::SetTextRenderingParams({0.0f, 0.75f, mode, Filter::kFreeType});
        const auto actual = SkGraphics::GetTextRenderingParams();
        REPORTER_ASSERT(reporter, actual.fGamma == 0.0f);
        REPORTER_ASSERT(reporter, actual.fContrast == 0.75f);
        REPORTER_ASSERT(reporter, actual.fApplyToA8 == mode);
        REPORTER_ASSERT(reporter, actual.fPathLCDFilter == Filter::kFreeType);

        const SkSurfaceProps props;
        REPORTER_ASSERT(reporter, props.textGamma() == actual.fGamma);
        REPORTER_ASSERT(reporter, props.textContrast() == actual.fContrast);
        REPORTER_ASSERT(reporter, SkGraphics::ApplyGammaToA8(false) == (mode == A8::kEnabled));
        REPORTER_ASSERT(reporter, SkGraphics::ApplyGammaToA8(true) == (mode != A8::kDisabled));
    }
}

DEF_SERIAL_TEST(ScalerContext_LCDMaskPreBlend, reporter) {
    RestoreTextRenderingParams restore;
    constexpr int kSize = 8;
    // Distinct constant LUTs make channel selection observable after RGB565 packing.
    std::array<uint8_t, 256> identity, red, green, blue;
    for (int i = 0; i < 256; ++i) {
        identity[i] = i;
        red[i] = 64;
        green[i] = 128;
        blue[i] = 192;
    }

    SkMaskGamma::PreBlend identityBlend;
    identityBlend.fR = identityBlend.fG = identityBlend.fB = identity.data();
    SkMaskGamma::PreBlend colorBlend;
    colorBlend.fR = red.data();
    colorBlend.fG = green.data();
    colorBlend.fB = blue.data();

    const SkPath path = SkPath::Rect(SkRect::MakeLTRB(2.25f, 2.25f, 5.75f, 5.75f));
    for (auto filter : {SkGraphics::PathLCDFilter::kDefault,
                        SkGraphics::PathLCDFilter::kLegacy,
                        SkGraphics::PathLCDFilter::kFreeType}) {
        auto params = restore.saved;
        params.fPathLCDFilter = filter;
        SkGraphics::SetTextRenderingParams(params);
        for (bool vertical : {false, true}) {
            for (bool bgr : {false, true}) {
                std::array<uint16_t, kSize * kSize> raw, unchanged, corrected;
                auto render = [&](auto& pixels, const SkMaskGamma::PreBlend& blend) {
                    SkMaskBuilder mask(reinterpret_cast<uint8_t*>(pixels.data()),
                                       SkIRect::MakeWH(kSize, kSize),
                                       kSize * sizeof(uint16_t),
                                       SkMask::kLCD16_Format);
                    PathMaskScalerContext::GenerateImageFromPath(
                            mask, path, blend, bgr, vertical, false, false);
                };
                render(raw, SkMaskGamma::PreBlend());
                render(unchanged, identityBlend);
                render(corrected, colorBlend);
                REPORTER_ASSERT(reporter, raw == unchanged);
                REPORTER_ASSERT(reporter, raw != corrected);
                for (uint16_t pixel : corrected) {
                    REPORTER_ASSERT(reporter, pixel == SkPack888ToRGB16(64, 128, 192));
                }
            }
        }
    }
}

DEF_SERIAL_TEST(ScalerContext_LCDFilterSelection, reporter) {
    RestoreTextRenderingParams restore;
    constexpr int kSize = 8;
    const SkPath path = SkPath::Rect(SkRect::MakeLTRB(2.25f, 2.25f, 5.75f, 5.75f));
    auto render = [&](SkGraphics::PathLCDFilter filter, bool lcd, bool vertical) {
        auto params = restore.saved;
        params.fPathLCDFilter = filter;
        SkGraphics::SetTextRenderingParams(params);
        std::array<uint16_t, kSize * kSize> pixels = {};
        SkMaskBuilder mask(reinterpret_cast<uint8_t*>(pixels.data()),
                           SkIRect::MakeWH(kSize, kSize),
                           kSize * (lcd ? sizeof(uint16_t) : sizeof(uint8_t)),
                           lcd ? SkMask::kLCD16_Format : SkMask::kA8_Format);
        PathMaskScalerContext::GenerateImageFromPath(
                mask, path, SkMaskGamma::PreBlend(), false, vertical, false, false);
        return pixels;
    };
    for (bool vertical : {false, true}) {
        const auto legacy = render(SkGraphics::PathLCDFilter::kLegacy, true, vertical);
        const auto freetype = render(SkGraphics::PathLCDFilter::kFreeType, true, vertical);
        REPORTER_ASSERT(reporter, legacy != freetype);
        const auto defaults = render(SkGraphics::PathLCDFilter::kDefault, true, vertical);
#if defined(SK_USE_LCD_TEXT_3X_FILTER)
        REPORTER_ASSERT(reporter, defaults == freetype);
#else
        REPORTER_ASSERT(reporter, defaults == legacy);
#endif
        REPORTER_ASSERT(reporter,
                        render(SkGraphics::PathLCDFilter::kLegacy, false, vertical) ==
                                render(SkGraphics::PathLCDFilter::kFreeType, false, vertical));
    }
}
