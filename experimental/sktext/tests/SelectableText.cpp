// Copyright 2021 Google LLC.
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include "experimental/sktext/include/Text.h"
#include "experimental/sktext/src/Paint.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

#define DEF_TEST_DISABLED(name, reporter) \
static void test_##name(skiatest::Reporter* reporter, const GrContextOptions&); \
skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name, false, test_##name)); \
void test_##name(skiatest::Reporter* reporter, const GrContextOptions&) { /* SkDebugf("Disabled:"#name "\n"); */ } \
void disabled_##name(skiatest::Reporter* reporter, const GrContextOptions&)

using namespace skia::text;

namespace {
    bool operator==(SkSpan<const char16_t> a, SkSpan<const char16_t> b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
}

struct TestLine {
    size_t index;
    TextRange lineText;
    bool hardBreak;
    SkRect bounds;
    GlyphRange trailingSpaces;
    Range<RunIndex> runRange;
    size_t glyphCount;
};

struct TestRun {
    const SkFont& font;
    TextRange textRange;        // Currently we make sure that the run edges are the grapheme cluster edges
    SkRect bounds;              // bounds contains the physical boundaries of the run
    int trailingSpaces;         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
    SkSpan<const uint16_t> glyphs;
    SkSpan<const SkPoint> positions;
    SkSpan<const TextIndex> clusters;
};

class TestVisitor : public Visitor {
public:
    void onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) override {
        SkASSERT(fTestLines.size() == index);
        fTestLines.push_back({ index, lineText, hardBreak, bounds, EMPTY_RANGE, Range<RunIndex>(fTestRuns.size(), fTestRuns.size()) });
    }
    void onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) override {
        SkASSERT(fTestLines.size() == index + 1);
        fTestLines.back().trailingSpaces = trailingSpaces;
        fTestLines.back().runRange.fEnd = fTestRuns.size();
        fTestLines.back().glyphCount = glyphCount;
    }
    void onGlyphRun(const SkFont& font,
                    TextRange textRange,        // Currently we make sure that the run edges are the grapheme cluster edges
                    SkRect bounds,              // bounds contains the physical boundaries of the run
                    int trailingSpaces,         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
                    int glyphCount,             // Just the number of glyphs
                    const uint16_t glyphs[],
                    const SkPoint positions[],        // Positions relative to the line
                    const TextIndex clusters[]) override
    {
        fTestRuns.push_back({font, textRange, bounds, trailingSpaces,
                            SkSpan<const uint16_t>(&glyphs[0], glyphCount),
                            SkSpan<const SkPoint>(&positions[0], glyphCount + 1),
                            SkSpan<const TextIndex>(&clusters[0], glyphCount + 1),
                            });
    }
    void onPlaceholder(TextRange, const SkRect& bounds) override { }

    std::vector<TestLine> fTestLines;
    std::vector<TestRun> fTestRuns;
};

DEF_TEST(SkText_SelectableText_Bounds, reporter) {
    REPORTER_ASSERT(reporter, true);
    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));

    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    FontBlock fontBlock(utf16.size(), fontChain);
    auto shapedText = unicodeText.shape(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit();

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    REPORTER_ASSERT(reporter, selectableText->countLines() == 5);
    for (LineIndex lineIndex = 0; lineIndex < selectableText->countLines(); ++lineIndex) {
        auto& testLine = testVisitor.fTestLines[lineIndex];
        auto boxLine = selectableText->getLine(lineIndex);
        SkScalar left = boxLine.fBounds.fLeft;
        for (auto& box : boxLine.fBoxGlyphs) {
            REPORTER_ASSERT(reporter, boxLine.fBounds.contains(box));
            REPORTER_ASSERT(reporter, left <= box.fLeft);
            left = box.fRight;
        }

        GlyphIndex trailingSpaces = boxLine.fBoxGlyphs.size();
        for (RunIndex runIndex = testLine.runRange.fEnd; runIndex > testLine.runRange.fStart; --runIndex) {
            auto& testRun = testVisitor.fTestRuns[runIndex - 1];
            if (testRun.trailingSpaces == 0) {
                trailingSpaces -= testRun.glyphs.size();
            } else {
                trailingSpaces -= (testRun.glyphs.size() - testRun.trailingSpaces);
                break;
            }
        }

        REPORTER_ASSERT(reporter, boxLine.fTrailingSpacesEnd == testLine.trailingSpaces.fEnd);
        REPORTER_ASSERT(reporter, boxLine.fTextEnd == trailingSpaces);
        REPORTER_ASSERT(reporter, boxLine.fTextRange == testLine.lineText);
        REPORTER_ASSERT(reporter, boxLine.fIndex == lineIndex);
        REPORTER_ASSERT(reporter, boxLine.fIsHardBreak == testLine.hardBreak);
        REPORTER_ASSERT(reporter, boxLine.fBounds == testLine.bounds);
    }
}

DEF_TEST(SkText_SelectableText_Navigation_FirstLast, reporter) {
    REPORTER_ASSERT(reporter, true);
    std::u16string utf16(u"    Leading spaces\nTrailing spaces    \nLong text with collapsed      spaces inside wrapped into few lines");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));

    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    FontBlock fontBlock(utf16.size(), fontChain);
    auto shapedText = unicodeText.shape(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit();

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);

    // First position
    auto firstLine = testVisitor.fTestLines.front();
    auto firstRun = testVisitor.fTestRuns.front();
    auto firstPosition = selectableText->firstPosition(PositionType::kGraphemeCluster);
    REPORTER_ASSERT(reporter, firstPosition.fLineIndex == 0);
    REPORTER_ASSERT(reporter, firstPosition.fTextRange == TextRange(0, 0));
    REPORTER_ASSERT(reporter, firstPosition.fGlyphRange == GlyphRange(0, 0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fLeft, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fTop, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.height(), firstLine.bounds.height()));

    // Last position
    auto lastLine = testVisitor.fTestLines.back();
    auto lastRun = testVisitor.fTestRuns.back();
    auto lastPosition = selectableText->lastPosition(PositionType::kGraphemeCluster);
    REPORTER_ASSERT(reporter, lastPosition.fLineIndex == testVisitor.fTestLines.size() - 1);
    REPORTER_ASSERT(reporter, lastPosition.fTextRange == TextRange(utf16.size(), utf16.size()));
    REPORTER_ASSERT(reporter, lastPosition.fGlyphRange == GlyphRange(lastRun.glyphs.size(), lastRun.glyphs.size()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fLeft, lastRun.positions[lastRun.glyphs.size()].fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fTop, lastLine.bounds.fTop));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.height(), lastLine.bounds.height()));
}

DEF_TEST(SkText_SelectableText_NavigationByGraphemeClusters, reporter) {
    REPORTER_ASSERT(reporter, true);
    std::u16string utf16(u"    Small Text   \n");
    UnicodeText unicodeText(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));

    sk_sp<TrivialFontChain> fontChain = sk_make_sp<TrivialFontChain>("Roboto", 40.0f, SkFontStyle::Normal());
    FontBlock fontBlock(utf16.size(), fontChain);
    auto shapedText = unicodeText.shape(SkSpan<FontBlock>(&fontBlock, 1), TextDirection::kLtr);
    auto wrappedText = shapedText->wrap(&unicodeText, 440.0f, 500.0f);
    auto selectableText = wrappedText->prepareToEdit();

    TestVisitor testVisitor;
    wrappedText->visit(&testVisitor);


    // Start from the first position
    auto firstLine = testVisitor.fTestLines.front();
    auto firstRun = testVisitor.fTestRuns.front();
    auto firstPosition = selectableText->firstPosition(PositionType::kGraphemeCluster);

    while (true) {

    }
    REPORTER_ASSERT(reporter, firstPosition.fLineIndex == 0);
    REPORTER_ASSERT(reporter, firstPosition.fTextRange == TextRange(0, 0));
    REPORTER_ASSERT(reporter, firstPosition.fGlyphRange == GlyphRange(0, 0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fLeft, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.fTop, 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(firstPosition.fBoundaries.height(), firstLine.bounds.height()));

    // Last position
    auto lastLine = testVisitor.fTestLines.back();
    auto lastRun = testVisitor.fTestRuns.back();
    auto lastPosition = selectableText->lastPosition(PositionType::kGraphemeCluster);
    REPORTER_ASSERT(reporter, lastPosition.fLineIndex == testVisitor.fTestLines.size() - 1);
    REPORTER_ASSERT(reporter, lastPosition.fTextRange == TextRange(utf16.size(), utf16.size()));
    REPORTER_ASSERT(reporter, lastPosition.fGlyphRange == GlyphRange(lastRun.glyphs.size(), lastRun.glyphs.size()));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fLeft, lastRun.positions[lastRun.glyphs.size()].fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.fTop, lastLine.bounds.fTop));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.width(), 0.0f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(lastPosition.fBoundaries.height(), lastLine.bounds.height()));
}
