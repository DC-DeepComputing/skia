/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkExecutors.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkZip.h"
#include "src/text/StrikeForGPU.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <vector>

using namespace skia_private;

using namespace sktext;
using namespace skglyph;

class Barrier {
public:
    Barrier(int threadCount) : fThreadCount(threadCount) { }
    void waitForAll() {
        fThreadCount -= 1;
        while (fThreadCount > 0) { }
    }

private:
    std::atomic<int> fThreadCount;
};

// This should stay in sync with the implementation from SubRunContainer.
static
std::tuple<SkZip<const SkPackedGlyphID, const SkPoint>,
        SkZip<SkGlyphID, SkPoint>,
        SkRect>
prepare_for_mask_drawing(
        StrikeForGPU* strike,
        SkZip<const SkGlyphID, const SkPoint> source,
        SkZip<SkPackedGlyphID, SkPoint> acceptedBuffer,
        SkZip<SkGlyphID, SkPoint> rejectedBuffer) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    int acceptedSize = 0,
        rejectedSize = 0;
    StrikeMutationMonitor m{strike};
    for (auto [glyphID, pos] : source) {
        if (!SkIsFinite(pos.x(), pos.y())) {
            continue;
        }
        const SkPackedGlyphID packedID{glyphID};
        switch (const SkGlyphDigest digest = strike->digestFor(kDirectMask, packedID);
                digest.actionFor(kDirectMask)) {
            case GlyphAction::kAccept: {
                const SkGlyphRect glyphBounds = digest.bounds().offset(pos);
                boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                acceptedBuffer[acceptedSize++] = std::make_tuple(packedID, glyphBounds.leftTop());
                break;
            }
            case GlyphAction::kReject:
                rejectedBuffer[rejectedSize++] = std::make_tuple(glyphID, pos);
                break;
            default:
                break;
        }
    }

    return {acceptedBuffer.first(acceptedSize),
            rejectedBuffer.first(rejectedSize),
            boundingRect.rect()};
}

DEF_TEST(SkStrikeMultiThread, Reporter) {
    sk_sp<SkTypeface> typeface =
            ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Italic());
    static constexpr int kThreadCount = 4;

    Barrier barrier{kThreadCount};

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);
    font.setTypeface(typeface);

    std::array<SkGlyphID, 'z'> glyphs;
    std::array<SkPoint, 'z'> pos;
    for (int c = ' '; c < 'z'; c++) {
        glyphs[c] = font.unicharToGlyph(c);
        pos[c] = {30.0f * c + 30, 30.0f};
    }
    constexpr size_t glyphCount = 'z' - ' ';
    auto data = SkMakeZip(glyphs, pos).subspan(SkTo<int>(' '), glyphCount);

    SkPaint defaultPaint;
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
            font, defaultPaint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
            SkScalerContextFlags::kNone, SkMatrix::I());

    SkStrikeCache strikeCache;

    // Make our own executor so the --threads parameter doesn't mess things up.
    auto executor = SkFunctionalExecutors::MakeFIFOThreadPool(kThreadCount);
    for (int tries = 0; tries < 100; tries++) {
        SkStrike strike{&strikeCache, strikeSpec, strikeSpec.createScalerContext(), nullptr,
                        nullptr};

        SkTaskGroup taskGroup(*executor);
        for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex) {
            taskGroup.add([&, threadIndex]() {
                barrier.waitForAll();

                auto local = data.subspan(threadIndex * 2, data.size() - kThreadCount * 2);
                for (int i = 0; i < 100; i++) {
                    // Accepted buffers.
                    STArray<64, SkPackedGlyphID> acceptedPackedGlyphIDs;
                    STArray<64, SkPoint> acceptedPositions;
                    STArray<64, SkMask::Format> acceptedFormats;
                    acceptedPackedGlyphIDs.resize(glyphCount);
                    acceptedPositions.resize(glyphCount);
                    const auto acceptedBuffer = SkMakeZip(acceptedPackedGlyphIDs, acceptedPositions);

                    // Rejected buffers.
                    STArray<64, SkGlyphID> rejectedGlyphIDs;
                    STArray<64, SkPoint> rejectedPositions;
                    rejectedGlyphIDs.resize(glyphCount);
                    rejectedPositions.resize(glyphCount);
                    const auto rejectedBuffer = SkMakeZip(rejectedGlyphIDs, rejectedPositions);

                    SkZip<const SkGlyphID, const SkPoint> source = local;

                    auto [accepted, rejected, bounds] =
                        prepare_for_mask_drawing(&strike, source, acceptedBuffer, rejectedBuffer);
                    source = rejected;
                }
            });
        }
        taskGroup.wait();
    }
}

class SkGlyphTestPeer {
public:
    static void SetGlyph(SkGlyph* glyph) {
        // Tweak the bounds to make them unique based on glyph id.
        const SkGlyphID uniquify = glyph->getGlyphID();
        glyph->fAdvanceX = 10;
        glyph->fAdvanceY = 11;
        glyph->fLeft = -1 - uniquify;
        glyph->fTop = -2;
        glyph->fWidth = 8;
        glyph->fHeight = 9;
        glyph->fMaskFormat = SkMask::Format::kA8_Format;
    }
};

class SkStrikeTestingPeer {
public:
    static SkGlyph* GetGlyph(SkStrike* strike, SkPackedGlyphID packedID) {
        SkAutoMutexExclusive m{strike->fStrikeLock};
        return strike->glyph(packedID);
    }
};

DEF_TEST(SkStrike_FlattenByType, reporter) {
    std::vector<SkGlyph> imagesToSend;
    std::vector<SkGlyph> pathsToSend;
    std::vector<SkGlyph> drawablesToSend;
    SkArenaAlloc alloc{256};

    // Make a mask glyph and put it in the glyphs to send.
    const SkPackedGlyphID maskPackedGlyphID((SkGlyphID)10);
    SkGlyph maskGlyph{maskPackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&maskGlyph);

    static constexpr uint8_t X = 0xff;
    static constexpr uint8_t O = 0x00;
    uint8_t imageData[][8] = {
            {X,X,X,X,X,X,X,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,X,X,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,O,O,O,O,O,O,X},
            {X,X,X,X,X,X,X,X},
    };
    maskGlyph.setImage(&alloc, imageData);
    imagesToSend.emplace_back(maskGlyph);

    // Make a path glyph and put it in the glyphs to send.
    const SkPackedGlyphID pathPackedGlyphID((SkGlyphID)11);
    SkGlyph pathGlyph{pathPackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&pathGlyph);
    SkPath path = SkPath::Rect(pathGlyph.rect());
    pathGlyph.setPath(&alloc, &path, false, false);
    pathsToSend.emplace_back(pathGlyph);

    // Make a drawable glyph and put it in the glyphs to send.
    const SkPackedGlyphID drawablePackedGlyphID((SkGlyphID)12);
    SkGlyph drawableGlyph{drawablePackedGlyphID};
    SkGlyphTestPeer::SetGlyph(&drawableGlyph);
    class TestDrawable final : public SkDrawable {
    public:
        TestDrawable(SkRect rect) : fRect(rect) {}

    private:
        const SkRect fRect;
        SkRect onGetBounds() override { return fRect;  }
        size_t onApproximateBytesUsed() override {
            return 0;
        }
        void onDraw(SkCanvas* canvas) override {
            SkPaint paint;
            canvas->drawRect(fRect, paint);
        }
    };

    sk_sp<SkDrawable> drawable = sk_make_sp<TestDrawable>(drawableGlyph.rect());
    REPORTER_ASSERT(reporter, !drawableGlyph.setDrawableHasBeenCalled());
    drawableGlyph.setDrawable(&alloc, drawable);
    REPORTER_ASSERT(reporter, drawableGlyph.setDrawableHasBeenCalled());
    REPORTER_ASSERT(reporter, drawableGlyph.drawable() != nullptr);
    drawablesToSend.emplace_back(drawableGlyph);

    // Test the FlattenGlyphsByType method.
    SkBinaryWriteBuffer writeBuffer({});
    SkStrike::FlattenGlyphsByType(writeBuffer, imagesToSend, pathsToSend, drawablesToSend);
    auto data = writeBuffer.snapshotAsData();

    // Make a strike to merge into.
    SkStrikeCache strikeCache;
    auto dstTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    SkFont font{dstTypeface};
    SkStrikeSpec spec = SkStrikeSpec::MakeWithNoDevice(font);
    sk_sp<SkStrike> strike = spec.findOrCreateStrike(&strikeCache);

    // Test the mergeFromBuffer method.
    SkReadBuffer readBuffer{data->data(), data->size()};
    strike->mergeFromBuffer(readBuffer);

    // Check mask glyph.
    SkGlyph* dstMaskGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(), maskPackedGlyphID);
    REPORTER_ASSERT(reporter, maskGlyph.rect() == dstMaskGlyph->rect());
    REPORTER_ASSERT(reporter, dstMaskGlyph->setImageHasBeenCalled());
    REPORTER_ASSERT(reporter, dstMaskGlyph->image() != nullptr);

    // Check path glyph.
    SkGlyph* dstPathGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(), pathPackedGlyphID);
    REPORTER_ASSERT(reporter, pathGlyph.rect() == dstPathGlyph->rect());
    REPORTER_ASSERT(reporter, dstPathGlyph->setPathHasBeenCalled());
    REPORTER_ASSERT(reporter, dstPathGlyph->path() != nullptr);

    // Check drawable glyph.
    SkGlyph* dstDrawableGlyph = SkStrikeTestingPeer::GetGlyph(strike.get(),drawablePackedGlyphID);
    REPORTER_ASSERT(reporter, drawableGlyph.rect() == dstDrawableGlyph->rect());
    REPORTER_ASSERT(reporter, dstDrawableGlyph->setDrawableHasBeenCalled());
    REPORTER_ASSERT(reporter, dstDrawableGlyph->drawable() != nullptr);
}

DEF_TEST(SkScalerContext_GetImageNonDeterministicBounds, reporter) {
    class NonDeterministicTypeface final : public TestEmptyTypeface {
    public:
        explicit NonDeterministicTypeface(SkRect secondBounds) : fSecondBounds(secondBounds) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(NonDeterministicTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc,
                              SkRect secondBounds)
                        : SkScalerContext(tf, effects, desc), fSecondBounds(secondBounds) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph& glyph, SkArenaAlloc*) override {
                    GlyphMetrics mx(glyph.maskFormat());
                    // Force the raster getImage() path instead of drawing from a glyph path
                    mx.neverRequestPath = true;
                    // Return valid 10x10 bounds on initial makeGlyph(), then fSecondBounds when
                    // getImage() reloads the unfiltered glyph for mask filtering
                    mx.bounds =
                            (fMetricsCallCount++ == 0) ? SkRect::MakeWH(10, 10) : fSecondBounds;
                    return mx;
                }

                void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
                    // Intentionally use rowBytes() * height() like SkScalerContextFTUtils does;
                    // if getImage() fails to reject an imageTooLarge() or empty unfiltered glyph,
                    // this overflows the small origGlyph buffer or dereferences nullptr
                    memset(imageBuffer, 0xAA, glyph.rowBytes() * glyph.height());
                }

                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }

                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const SkRect fSecondBounds;
                int fMetricsCallCount = 0;
            };

            return std::make_unique<ScalerContext>(
                    *const_cast<NonDeterministicTypeface*>(this), effects, desc, fSecondBounds);
        }

    private:
        const SkRect fSecondBounds;
    };

    for (const SkRect& secondBounds : {
                 // Exceeds kMaxGlyphWidth so imageTooLarge() is true and imageSize() returns 0
                 SkRect::MakeWH(1 << 14, 10),
                 // Zero-area unfiltered glyph on the second load
                 SkRect::MakeEmpty(),
                 // Unfiltered bounds disjoint horizontally and vertically from the 10x10 dstMask
                 SkRect::MakeXYWH(500, 0, 10, 10),
                 SkRect::MakeXYWH(0, 500, 10, 10),
         }) {
        sk_sp<SkTypeface> tf = sk_make_sp<NonDeterministicTypeface>(secondBounds);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        // A mask filter triggers the second makeGlyph() call inside SkScalerContext::getImage
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.0f));

        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(
                font, paint, SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                SkScalerContextFlags::kNone, SkMatrix::I());
        std::unique_ptr<SkScalerContext> context = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = context->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, !glyph.isEmpty());
        REPORTER_ASSERT(reporter, !glyph.imageTooLarge());

        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, context.get()));
        REPORTER_ASSERT(reporter, glyph.image() != nullptr);

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }
}

DEF_TEST(SkScalerContext_GetImageNoOpFilterDivergentFormats, reporter) {
    class DivergentFormatTypeface final : public TestEmptyTypeface {
    public:
        DivergentFormatTypeface(SkRect firstBounds,
                                SkRect secondBounds,
                                SkMask::Format firstFormat,
                                SkMask::Format secondFormat)
                : fFirstBounds(firstBounds)
                , fSecondBounds(secondBounds)
                , fFirstFormat(firstFormat)
                , fSecondFormat(secondFormat) {}

    protected:
        std::unique_ptr<SkScalerContext> onCreateScalerContext(
                const SkScalerContextEffects& effects, const SkDescriptor* desc) const override {
            class ScalerContext final : public SkScalerContext {
            public:
                ScalerContext(DivergentFormatTypeface& tf,
                              const SkScalerContextEffects& effects,
                              const SkDescriptor* desc)
                        : SkScalerContext(tf, effects, desc), fOwner(tf) {}

            protected:
                GlyphMetrics generateMetrics(const SkGlyph&, SkArenaAlloc*) override {
                    const bool isFirst = (fMetricsCallCount++ == 0);
                    GlyphMetrics mx(isFirst ? fOwner.fFirstFormat : fOwner.fSecondFormat);
                    mx.neverRequestPath = true;
                    mx.bounds = isFirst ? fOwner.fFirstBounds : fOwner.fSecondBounds;
                    return mx;
                }

                void generateImage(const SkGlyph& glyph, void* imageBuffer) override {
                    switch (glyph.maskFormat()) {
                        case SkMask::kBW_Format:
                            memset(imageBuffer, 0xFF, glyph.imageSize());
                            break;
                        case SkMask::kLCD16_Format: {
                            for (int y = 0; y < glyph.height(); ++y) {
                                uint16_t* row = reinterpret_cast<uint16_t*>(
                                        static_cast<uint8_t*>(imageBuffer) + y * glyph.rowBytes());
                                for (int x = 0; x < glyph.width(); ++x) {
                                    row[x] = 0x1234;
                                }
                            }
                            break;
                        }
                        case SkMask::kARGB32_Format: {
                            for (int y = 0; y < glyph.height(); ++y) {
                                uint32_t* row = reinterpret_cast<uint32_t*>(
                                        static_cast<uint8_t*>(imageBuffer) + y * glyph.rowBytes());
                                for (int x = 0; x < glyph.width(); ++x) {
                                    row[x] = 0xFF112233;
                                }
                            }
                            break;
                        }
                        default:
                            memset(imageBuffer, 0x7F, glyph.imageSize());
                            break;
                    }
                }

                std::optional<GeneratedPath> generatePath(const SkGlyph&) override {
                    return std::nullopt;
                }

                void generateFontMetrics(SkFontMetrics*) override {}

            private:
                const DivergentFormatTypeface& fOwner;
                int fMetricsCallCount = 0;
            };

            return std::make_unique<ScalerContext>(
                    *const_cast<DivergentFormatTypeface*>(this), effects, desc);
        }

    private:
        const SkRect fFirstBounds;
        const SkRect fSecondBounds;
        const SkMask::Format fFirstFormat;
        const SkMask::Format fSecondFormat;
    };

    // A tiny positive sigma creates a non-null blur filter whose filterMask() returns false
    // (hasNoBlur()), exercising SkScalerContext::getImage()'s fallback intersection copy on
    // non-A8 mask formats.
    sk_sp<SkMaskFilter> noOpBlur = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.005f);
    REPORTER_ASSERT(reporter, noOpBlur != nullptr);

    // 1. kBW_Format (1 bit/pixel): 8x2 (2-byte buffer) shifted by (+3, +1) and (-3, -1).
    for (bool shiftPositive : {true, false}) {
        const SkRect second =
                shiftPositive ? SkRect::MakeXYWH(3, 1, 8, 2) : SkRect::MakeXYWH(-3, -1, 8, 2);
        sk_sp<SkTypeface> tf = sk_make_sp<DivergentFormatTypeface>(
                SkRect::MakeWH(8, 2), second, SkMask::kBW_Format, SkMask::kBW_Format);
        SkFont font(tf, 16.0f);
        font.setEdging(SkFont::Edging::kAlias);
        SkPaint paint;
        paint.setMaskFilter(noOpBlur);

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kBW_Format);
        REPORTER_ASSERT(reporter, glyph.imageSize() == 2);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        if (shiftPositive) {
            REPORTER_ASSERT(reporter, bytes[0] == 0x00);
            REPORTER_ASSERT(reporter, bytes[1] == 0x1F);
        } else {
            REPORTER_ASSERT(reporter, bytes[0] == 0xF8);
            REPORTER_ASSERT(reporter, bytes[1] == 0x00);
        }
    }

    // 2. kLCD16_Format (2 bytes/pixel): odd horizontal pixel shift (+3, +1) must preserve 16-bit
    // pixel alignment rather than shifting by 3 bytes.
    {
        sk_sp<SkTypeface> tf = sk_make_sp<DivergentFormatTypeface>(SkRect::MakeWH(6, 3),
                                                                   SkRect::MakeXYWH(3, 1, 6, 3),
                                                                   SkMask::kLCD16_Format,
                                                                   SkMask::kLCD16_Format);
        SkFont font(tf, 16.0f);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        SkPaint paint;
        paint.setMaskFilter(noOpBlur);

        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeMask(font,
                                                         paint,
                                                         SkSurfaceProps(0, kRGB_H_SkPixelGeometry),
                                                         SkScalerContextFlags::kNone,
                                                         SkMatrix::I());
        std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kLCD16_Format);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

        const uint8_t* base = static_cast<const uint8_t*>(glyph.image());
        for (int y = 0; y < glyph.height(); ++y) {
            const uint16_t* row = reinterpret_cast<const uint16_t*>(base + y * glyph.rowBytes());
            for (int x = 0; x < glyph.width(); ++x) {
                const uint16_t expected = (x >= 3 && y >= 1) ? 0x1234 : 0x0000;
                REPORTER_ASSERT(reporter, row[x] == expected);
            }
        }
    }

    // 3. kARGB32_Format (4 bytes/pixel): (+3, +1) shift must copy 4 bytes per pixel.
    {
        sk_sp<SkTypeface> tf = sk_make_sp<DivergentFormatTypeface>(SkRect::MakeWH(6, 3),
                                                                   SkRect::MakeXYWH(3, 1, 6, 3),
                                                                   SkMask::kARGB32_Format,
                                                                   SkMask::kARGB32_Format);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setMaskFilter(noOpBlur);

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kARGB32_Format);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

        const uint8_t* base = static_cast<const uint8_t*>(glyph.image());
        for (int y = 0; y < glyph.height(); ++y) {
            const uint32_t* row = reinterpret_cast<const uint32_t*>(base + y * glyph.rowBytes());
            for (int x = 0; x < glyph.width(); ++x) {
                const uint32_t expected = (x >= 3 && y >= 1) ? 0xFF112233u : 0u;
                REPORTER_ASSERT(reporter, row[x] == expected);
            }
        }
    }

    // 4. Format mismatch across loads (Load 1 kARGB32_Format vs Load 2 kA8_Format) must clear
    // origGlyph's image rather than crashing or copying mismatched pixel strides.
    {
        sk_sp<SkTypeface> tf = sk_make_sp<DivergentFormatTypeface>(SkRect::MakeWH(6, 3),
                                                                   SkRect::MakeWH(6, 3),
                                                                   SkMask::kARGB32_Format,
                                                                   SkMask::kA8_Format);
        SkFont font(tf, 16.0f);
        SkPaint paint;
        paint.setMaskFilter(noOpBlur);

        SkStrikeSpec strikeSpec =
                SkStrikeSpec::MakeMask(font,
                                       paint,
                                       SkSurfaceProps(0, kUnknown_SkPixelGeometry),
                                       SkScalerContextFlags::kNone,
                                       SkMatrix::I());
        std::unique_ptr<SkScalerContext> ctx = strikeSpec.createScalerContext();

        SkArenaAlloc alloc{256};
        SkGlyph glyph = ctx->makeGlyph(SkPackedGlyphID((SkGlyphID)1), &alloc);
        REPORTER_ASSERT(reporter, glyph.maskFormat() == SkMask::kARGB32_Format);
        REPORTER_ASSERT(reporter, glyph.setImage(&alloc, ctx.get()));

        const uint8_t* bytes = static_cast<const uint8_t*>(glyph.image());
        for (size_t i = 0; i < glyph.imageSize(); ++i) {
            REPORTER_ASSERT(reporter, bytes[i] == 0);
        }
    }
}
