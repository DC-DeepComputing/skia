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
#include "include/core/SkSpan.h"
#include "include/core/SkStrikeRef.h"
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

namespace {

constexpr SkScalar kStrikeRefSentinel = -12345.0f;

// Returns glyphs with (likely) differing advances, so mixed-up slots are detectable.
std::array<SkGlyphID, 4> strike_ref_test_glyphs(const SkFont& font) {
    return {font.unicharToGlyph('W'),
            font.unicharToGlyph('i'),
            font.unicharToGlyph('l'),
            font.unicharToGlyph('.')};
}

}  // namespace

// Regression test for a bug in an earlier spanified version of getWidthsStrided(): it advanced
// its input/output spans with subspan(stride) after the last element, which trapped on buffers
// sized exactly (count-1)*stride+1 whenever stride > 1.
DEF_TEST(SkStrikeRef_GetWidthsStrided_TightBuffers, reporter) {
    SkFont font = ToolUtils::DefaultPortableFont();
    SkStrikeRef strike = font.makeStrikeRef();
    if (!strike) {
        ERRORF(reporter, "SkFont::makeStrikeRef() returned an empty SkStrikeRef");
        return;
    }

    const std::array<SkGlyphID, 4> glyphs = strike_ref_test_glyphs(font);
    std::array<SkScalar, 4> expected;
    strike.getWidths(glyphs, expected);

    constexpr unsigned kCount = 4;
    constexpr unsigned kGlyphStride = 3;
    constexpr unsigned kAdvanceStride = 2;
    // Exactly (count-1)*stride+1 elements: no slack after the last element.
    std::array<uint32_t, (kCount - 1) * kGlyphStride + 1> glyphBuf{};
    std::array<SkScalar, (kCount - 1) * kAdvanceStride + 1> advanceBuf;
    advanceBuf.fill(kStrikeRefSentinel);
    for (unsigned i = 0; i < kCount; ++i) {
        glyphBuf[i * kGlyphStride] = glyphs[i];
    }

    strike.getWidthsStrided(kCount, glyphBuf, kGlyphStride, advanceBuf, kAdvanceStride);

    for (size_t i = 0; i < advanceBuf.size(); ++i) {
        if (i % kAdvanceStride == 0) {
            REPORTER_ASSERT(reporter,
                            advanceBuf[i] == expected[i / kAdvanceStride],
                            "slot %zu: got %f, expected %f",
                            i,
                            advanceBuf[i],
                            expected[i / kAdvanceStride]);
        } else {
            // Slots between strides must not be written.
            REPORTER_ASSERT(
                    reporter, advanceBuf[i] == kStrikeRefSentinel, "gap slot %zu was written", i);
        }
    }
}

DEF_TEST(SkStrikeRef_GetWidthsStrided_ZeroStride, reporter) {
    SkFont font = ToolUtils::DefaultPortableFont();
    SkStrikeRef strike = font.makeStrikeRef();
    if (!strike) {
        ERRORF(reporter, "SkFont::makeStrikeRef() returned an empty SkStrikeRef");
        return;
    }

    const std::array<SkGlyphID, 4> glyphs = strike_ref_test_glyphs(font);
    std::array<SkScalar, 4> expected;
    strike.getWidths(glyphs, expected);

    // A stride of 0 reads the same glyph and writes the same slot every iteration, so
    // single-element buffers are sufficient.
    std::array<uint32_t, 1> glyphBuf = {glyphs[0]};
    std::array<SkScalar, 1> advanceBuf = {kStrikeRefSentinel};
    strike.getWidthsStrided(3, glyphBuf, 0, advanceBuf, 0);
    REPORTER_ASSERT(reporter, advanceBuf[0] == expected[0]);
}

DEF_TEST(SkStrikeRef_GetWidthsStrided_ZeroCount, reporter) {
    SkFont font = ToolUtils::DefaultPortableFont();
    SkStrikeRef strike = font.makeStrikeRef();
    if (!strike) {
        ERRORF(reporter, "SkFont::makeStrikeRef() returned an empty SkStrikeRef");
        return;
    }

    // count == 0 must not touch (or require any space in) either buffer.
    strike.getWidthsStrided(0, SkSpan<const uint32_t>(), 4, SkSpan<SkScalar>(), 4);

    std::array<SkScalar, 1> advanceBuf = {kStrikeRefSentinel};
    strike.getWidthsStrided(0, SkSpan<const uint32_t>(), 4, advanceBuf, 4);
    REPORTER_ASSERT(reporter, advanceBuf[0] == kStrikeRefSentinel);
}

// The deprecated pointer overload must forward to the SkSpan overload with the same results.
DEF_TEST(SkStrikeRef_GetWidthsStrided_LegacyPointerOverload, reporter) {
    SkFont font = ToolUtils::DefaultPortableFont();
    SkStrikeRef strike = font.makeStrikeRef();
    if (!strike) {
        ERRORF(reporter, "SkFont::makeStrikeRef() returned an empty SkStrikeRef");
        return;
    }

    const std::array<SkGlyphID, 4> glyphs = strike_ref_test_glyphs(font);
    std::array<SkScalar, 4> expected;
    strike.getWidths(glyphs, expected);

    constexpr unsigned kCount = 4;
    constexpr unsigned kGlyphStride = 2;
    constexpr unsigned kAdvanceStride = 3;
    std::array<uint32_t, (kCount - 1) * kGlyphStride + 1> glyphBuf{};
    std::array<SkScalar, (kCount - 1) * kAdvanceStride + 1> advanceBuf;
    advanceBuf.fill(kStrikeRefSentinel);
    for (unsigned i = 0; i < kCount; ++i) {
        glyphBuf[i * kGlyphStride] = glyphs[i];
    }

    // The buffers hold exactly (count-1)*stride+1 elements, as the pointer overload requires.
    strike.getWidthsStrided(
            kCount, glyphBuf.data(), kGlyphStride, advanceBuf.data(), kAdvanceStride);

    for (unsigned i = 0; i < kCount; ++i) {
        REPORTER_ASSERT(reporter, advanceBuf[i * kAdvanceStride] == expected[i]);
    }
}
