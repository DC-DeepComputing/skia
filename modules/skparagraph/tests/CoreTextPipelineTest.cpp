/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"
#include "modules/skparagraph/src/CoreTextLineShaper.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <string.h>

using namespace skia::textlayout;

namespace {
sk_sp<SkUnicode> get_unicode() {
    auto factory = SkShapers::BestAvailable();
    return sk_ref_sp<SkUnicode>(factory->getUnicode());
}
}  // namespace

DEF_TEST(CoreTextPipeline_IndexMap_AsciiAndMultibyte, reporter) {
    CoreTextIndexMap emptyMap("", 0);
    REPORTER_ASSERT(reporter, emptyMap.empty());
    REPORTER_ASSERT(reporter, emptyMap.utf8Length() == 0);
    REPORTER_ASSERT(reporter, emptyMap.utf16Length() == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf16Index(0) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf8Index(0) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf16Index(10) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf8Index(10) == 0);

    // Validate fast path for single-byte encoding and range roundtrips
    const char* ascii = "Hello, world!";
    size_t asciiLen = strlen(ascii);
    CoreTextIndexMap asciiMap(ascii, asciiLen);
    REPORTER_ASSERT(reporter, !asciiMap.empty());
    REPORTER_ASSERT(reporter, asciiMap.utf8Length() == asciiLen);
    REPORTER_ASSERT(reporter, asciiMap.utf16Length() == asciiLen);
    for (size_t i = 0; i <= asciiLen; ++i) {
        REPORTER_ASSERT(reporter, asciiMap.toUtf16Index(i) == i);
        REPORTER_ASSERT(reporter, asciiMap.toUtf8Index(i) == i);
    }
    CFRange cfRange = asciiMap.utf8RangeToCFRange(TextRange(0, 5));
    REPORTER_ASSERT(reporter, cfRange.location == 0 && cfRange.length == 5);
    TextRange u8Range = asciiMap.cfrangeToTextRange(cfRange);
    REPORTER_ASSERT(reporter, u8Range.start == 0 && u8Range.end == 5);

    // Cyrillic code points expand to 2 bytes in UTF-8 but 1 code unit in UTF-16
    const char* cyrillic = "Привет";
    size_t cyrBytes = strlen(cyrillic);
    REPORTER_ASSERT(reporter, cyrBytes == 12);
    CoreTextIndexMap cyrMap(cyrillic, cyrBytes);
    REPORTER_ASSERT(reporter, cyrMap.utf8Length() == 12);
    REPORTER_ASSERT(reporter, cyrMap.utf16Length() == 6);
    for (size_t u16 = 0; u16 < 6; ++u16) {
        REPORTER_ASSERT(reporter, cyrMap.toUtf8Index(u16) == u16 * 2);
        REPORTER_ASSERT(reporter, cyrMap.toUtf16Index(u16 * 2) == u16);
        REPORTER_ASSERT(reporter, cyrMap.toUtf16Index(u16 * 2 + 1) == u16);
    }
    CFRange cyrCf = cyrMap.utf8RangeToCFRange(TextRange(0, 4));
    REPORTER_ASSERT(reporter, cyrCf.location == 0 && cyrCf.length == 2);
    TextRange cyrBack = cyrMap.cfrangeToTextRange(cyrCf);
    REPORTER_ASSERT(reporter, cyrBack.start == 0 && cyrBack.end == 4);

    const char* cjk = "日本語";
    size_t cjkBytes = strlen(cjk);
    REPORTER_ASSERT(reporter, cjkBytes == 9);
    CoreTextIndexMap cjkMap(cjk, cjkBytes);
    REPORTER_ASSERT(reporter, cjkMap.utf8Length() == 9);
    REPORTER_ASSERT(reporter, cjkMap.utf16Length() == 3);
    for (size_t u16 = 0; u16 < 3; ++u16) {
        REPORTER_ASSERT(reporter, cjkMap.toUtf8Index(u16) == u16 * 3);
        REPORTER_ASSERT(reporter, cjkMap.toUtf16Index(u16 * 3) == u16);
        REPORTER_ASSERT(reporter, cjkMap.toUtf16Index(u16 * 3 + 1) == u16);
        REPORTER_ASSERT(reporter, cjkMap.toUtf16Index(u16 * 3 + 2) == u16);
    }
}

DEF_TEST(CoreTextPipeline_IndexMap_EmojiAndSurrogatePairs, reporter) {
    // Emoji code points require 4 UTF-8 bytes and surrogate pairs in UTF-16
    const char* singleEmoji = "😀";
    size_t emojiBytes = strlen(singleEmoji);
    REPORTER_ASSERT(reporter, emojiBytes == 4);
    CoreTextIndexMap emojiMap(singleEmoji, emojiBytes);
    REPORTER_ASSERT(reporter, emojiMap.utf8Length() == 4);
    REPORTER_ASSERT(reporter, emojiMap.utf16Length() == 2);

    for (size_t b = 0; b < 4; ++b) {
        REPORTER_ASSERT(reporter, emojiMap.toUtf16Index(b) == 0);
    }
    REPORTER_ASSERT(reporter, emojiMap.toUtf16Index(4) == 2);
    REPORTER_ASSERT(reporter, emojiMap.toUtf8Index(0) == 0);
    REPORTER_ASSERT(reporter, emojiMap.toUtf8Index(1) == 0);
    REPORTER_ASSERT(reporter, emojiMap.toUtf8Index(2) == 4);

    CFRange cfEmoji = emojiMap.utf8RangeToCFRange(TextRange(0, 4));
    REPORTER_ASSERT(reporter, cfEmoji.location == 0 && cfEmoji.length == 2);
    TextRange u8Emoji = emojiMap.cfrangeToTextRange(cfEmoji);
    REPORTER_ASSERT(reporter, u8Emoji.start == 0 && u8Emoji.end == 4);

    const char* mixed = "A😀B";
    size_t mixedBytes = strlen(mixed);
    REPORTER_ASSERT(reporter, mixedBytes == 6);
    CoreTextIndexMap mixedMap(mixed, mixedBytes);
    REPORTER_ASSERT(reporter, mixedMap.utf8Length() == 6);
    REPORTER_ASSERT(reporter, mixedMap.utf16Length() == 4);

    REPORTER_ASSERT(reporter, mixedMap.toUtf16Index(0) == 0);
    REPORTER_ASSERT(reporter, mixedMap.toUtf16Index(1) == 1);
    REPORTER_ASSERT(reporter, mixedMap.toUtf16Index(5) == 3);
    REPORTER_ASSERT(reporter, mixedMap.toUtf16Index(6) == 4);

    REPORTER_ASSERT(reporter, mixedMap.toUtf8Index(0) == 0);
    REPORTER_ASSERT(reporter, mixedMap.toUtf8Index(1) == 1);
    REPORTER_ASSERT(reporter, mixedMap.toUtf8Index(2) == 1);
    REPORTER_ASSERT(reporter, mixedMap.toUtf8Index(3) == 5);
    REPORTER_ASSERT(reporter, mixedMap.toUtf8Index(4) == 6);

    CFRange cfB = mixedMap.utf8RangeToCFRange(TextRange(5, 6));
    REPORTER_ASSERT(reporter, cfB.location == 3 && cfB.length == 1);
    TextRange u8B = mixedMap.cfrangeToTextRange(cfB);
    REPORTER_ASSERT(reporter, u8B.start == 5 && u8B.end == 6);

    // Verify dynamic buffer expansion when input exceeds the 256-element stack threshold
    SkString manyEmojis;
    constexpr int kEmojiCount = 100;
    for (int i = 0; i < kEmojiCount; ++i) {
        manyEmojis.append(singleEmoji);
    }
    REPORTER_ASSERT(reporter, manyEmojis.size() == 400);
    CoreTextIndexMap heapMap(manyEmojis.c_str(), manyEmojis.size());
    REPORTER_ASSERT(reporter, heapMap.utf8Length() == 400);
    REPORTER_ASSERT(reporter, heapMap.utf16Length() == 200);
    for (int i = 0; i < kEmojiCount; ++i) {
        REPORTER_ASSERT(reporter, heapMap.toUtf16Index(i * 4) == static_cast<size_t>(i * 2));
        REPORTER_ASSERT(reporter, heapMap.toUtf8Index(i * 2) == static_cast<size_t>(i * 4));
    }
}

DEF_TEST(CoreTextPipeline_FontRegistry_DescriptorCacheAndFallback, reporter) {
    // Apple system font detection and descriptor caching
    CoreTextFontRegistry& registry = CoreTextFontRegistry::Instance();
    registry.clearCache();

    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(static_cast<const SkTypeface*>(nullptr)));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".AppleSystemUIFont"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".SF NS"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont("SF Pro"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont("system-ui"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont("-apple-system"));
    REPORTER_ASSERT(reporter, !CoreTextFontRegistry::isAppleSystemFont("CourierNewCustom"));

    auto sysFont = registry.createFont(nullptr, 18.0f);
    REPORTER_ASSERT(reporter, sysFont != nullptr);
    REPORTER_ASSERT(reporter, CTFontGetSize(sysFont.get()) == 18.0f);

    auto tf = ToolUtils::DefaultPortableTypeface();
    if (tf) {
        auto desc1 = registry.getDescriptor(tf.get());
        REPORTER_ASSERT(reporter, desc1 != nullptr);
        auto desc2 = registry.getDescriptor(tf.get());
        REPORTER_ASSERT(reporter, desc2 != nullptr);
        registry.clearCache();
        auto desc3 = registry.getDescriptor(tf.get());
        REPORTER_ASSERT(reporter, desc3 != nullptr);
    }
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_BaseDirectionBiDi, reporter) {
    // CoreText paragraph style must encode LTR and RTL base embedding direction
    const char* ltrText = "Hello world";
    size_t ltrLen = strlen(ltrText);
    CoreTextAttrStringBuilder ltrBuilder(ltrText, ltrLen, TextDirection::kLtr, nullptr);
    TextStyle ltrStyle;
    ltrBuilder.addStyle(TextRange(0, ltrLen), ltrStyle);
    auto ltrAttr = ltrBuilder.build();
    REPORTER_ASSERT(reporter, ltrAttr != nullptr);

    CTParagraphStyleRef ltrPStyle = (CTParagraphStyleRef)CFAttributedStringGetAttribute(
        ltrAttr.get(), 0, kCTParagraphStyleAttributeName, nullptr);
    REPORTER_ASSERT(reporter, ltrPStyle != nullptr);
    CTWritingDirection ltrDir = kCTWritingDirectionNatural;
    bool ok = CTParagraphStyleGetValueForSpecifier(
        ltrPStyle, kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(ltrDir), &ltrDir);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, ltrDir == kCTWritingDirectionLeftToRight);

    const char* rtlText = "سلام";
    size_t rtlLen = strlen(rtlText);
    CoreTextAttrStringBuilder rtlBuilder(rtlText, rtlLen, TextDirection::kRtl, nullptr);
    TextStyle rtlStyle;
    rtlBuilder.addStyle(TextRange(0, rtlLen), rtlStyle);
    auto rtlAttr = rtlBuilder.build();
    REPORTER_ASSERT(reporter, rtlAttr != nullptr);

    CTParagraphStyleRef rtlPStyle = (CTParagraphStyleRef)CFAttributedStringGetAttribute(
        rtlAttr.get(), 0, kCTParagraphStyleAttributeName, nullptr);
    REPORTER_ASSERT(reporter, rtlPStyle != nullptr);
    CTWritingDirection rtlDir = kCTWritingDirectionNatural;
    ok = CTParagraphStyleGetValueForSpecifier(
        rtlPStyle, kCTParagraphStyleSpecifierBaseWritingDirection, sizeof(rtlDir), &rtlDir);
    REPORTER_ASSERT(reporter, ok);
    REPORTER_ASSERT(reporter, rtlDir == kCTWritingDirectionRightToLeft);
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_OpenTypeFeatures, reporter) {
    // OpenType features must be packed into CTFontFeatureSettingsAttribute dictionary array
    const char* text = "Sample Features 1234";
    size_t textLen = strlen(text);
    CoreTextAttrStringBuilder builder(text, textLen, TextDirection::kLtr, nullptr);
    TextStyle style;
    style.addFontFeature(SkString("smcp"), 1);
    style.addFontFeature(SkString("tnum"), 1);
    builder.addStyle(TextRange(0, textLen), style);
    auto attrStr = builder.build();
    REPORTER_ASSERT(reporter, attrStr != nullptr);

    CTFontRef font = (CTFontRef)CFAttributedStringGetAttribute(
        attrStr.get(), 0, kCTFontAttributeName, nullptr);
    REPORTER_ASSERT(reporter, font != nullptr);

    SkUniqueCFRef<CFTypeRef> featAttr(CTFontCopyAttribute(font, kCTFontFeatureSettingsAttribute));
    REPORTER_ASSERT(reporter, featAttr != nullptr);
    CFArrayRef featArray = (CFArrayRef)featAttr.get();
    REPORTER_ASSERT(reporter, CFGetTypeID(featArray) == CFArrayGetTypeID());
    REPORTER_ASSERT(reporter, CFArrayGetCount(featArray) >= 2);
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_CursiveSpacingInvalidation, reporter) {
    // Invariant: letter spacing must not be applied to cursive scripts like Arabic
    const char* latin = "ABCDEF";
    size_t latinLen = strlen(latin);
    CoreTextAttrStringBuilder latinBuilder(latin, latinLen, TextDirection::kLtr, nullptr);
    TextStyle latinStyle;
    latinStyle.setLetterSpacing(3.5f);
    latinBuilder.addStyle(TextRange(0, latinLen), latinStyle);
    auto latinAttr = latinBuilder.build();
    REPORTER_ASSERT(reporter, latinAttr != nullptr);

    CFNumberRef kernNum = (CFNumberRef)CFAttributedStringGetAttribute(
        latinAttr.get(), 0, kCTKernAttributeName, nullptr);
    REPORTER_ASSERT(reporter, kernNum != nullptr);
    CGFloat kernVal = 0.0;
    CFNumberGetValue(kernNum, kCFNumberCGFloatType, &kernVal);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(kernVal, 3.5f));

    const char* arabic = "سلام";
    size_t arabicLen = strlen(arabic);
    CoreTextAttrStringBuilder arabicBuilder(arabic, arabicLen, TextDirection::kRtl, nullptr);
    TextStyle arabicStyle;
    arabicStyle.setLetterSpacing(3.5f);
    arabicBuilder.addStyle(TextRange(0, arabicLen), arabicStyle);
    auto arabicAttr = arabicBuilder.build();
    REPORTER_ASSERT(reporter, arabicAttr != nullptr);

    CFNumberRef arabicKern = (CFNumberRef)CFAttributedStringGetAttribute(
        arabicAttr.get(), 0, kCTKernAttributeName, nullptr);
    REPORTER_ASSERT(reporter, arabicKern == nullptr);

    const char* mixed = "Hi سلام Hi";
    size_t mixedLen = strlen(mixed);
    CoreTextAttrStringBuilder mixedBuilder(mixed, mixedLen, TextDirection::kLtr, nullptr);
    TextStyle mixedStyle;
    mixedStyle.setLetterSpacing(3.5f);
    mixedBuilder.addStyle(TextRange(0, mixedLen), mixedStyle);
    auto mixedAttr = mixedBuilder.build();
    REPORTER_ASSERT(reporter, mixedAttr != nullptr);

    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        mixedAttr.get(), 0, kCTKernAttributeName, nullptr) != nullptr);

    size_t u16ArabicIdx = mixedBuilder.indexMap().toUtf16Index(3);
    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        mixedAttr.get(), u16ArabicIdx, kCTKernAttributeName, nullptr) == nullptr);
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_HIGDynamicTracking, reporter) {
    // HIG tracking table yields negative tracking values for standard text sizes
    const char* text = "Dynamic tracking check";
    size_t textLen = strlen(text);
    CoreTextAttrStringBuilder builder(text, textLen, TextDirection::kLtr, nullptr);
    TextStyle style;
    style.setFontSize(16.0f);
    builder.addStyle(TextRange(0, textLen), style);
    auto attrStr = builder.build();
    REPORTER_ASSERT(reporter, attrStr != nullptr);

    CFStringRef trackingAttrKey = CFSTR("CTTracking");
    CFNumberRef trackingNum = (CFNumberRef)CFAttributedStringGetAttribute(
        attrStr.get(), 0, trackingAttrKey, nullptr);
    REPORTER_ASSERT(reporter, trackingNum != nullptr);
    CGFloat trackingVal = 0.0;
    CFNumberGetValue(trackingNum, kCFNumberCGFloatType, &trackingVal);
    REPORTER_ASSERT(reporter, trackingVal < 0.0f);
}

DEF_TEST(CoreTextPipeline_LineShapingAndBiDiLevels, reporter) {
    // CTLine separates runs by direction with correct kCTRunStatusRightToLeft flags
    const char* bidiText = "Hello سلام World";
    size_t bidiLen = strlen(bidiText);
    CoreTextAttrStringBuilder builder(bidiText, bidiLen, TextDirection::kLtr, nullptr);
    TextStyle style;
    style.setFontSize(14.0f);
    builder.addStyle(TextRange(0, bidiLen), style);
    auto attrStr = builder.build();
    REPORTER_ASSERT(reporter, attrStr != nullptr);

    SkUniqueCFRef<CTLineRef> line(CTLineCreateWithAttributedString(attrStr.get()));
    REPORTER_ASSERT(reporter, line != nullptr);

    CFArrayRef runs = CTLineGetGlyphRuns(line.get());
    REPORTER_ASSERT(reporter, runs != nullptr);
    CFIndex runCount = CFArrayGetCount(runs);
    REPORTER_ASSERT(reporter, runCount >= 3);

    CTRunRef run0 = (CTRunRef)CFArrayGetValueAtIndex(runs, 0);
    CTRunRef run1 = (CTRunRef)CFArrayGetValueAtIndex(runs, 1);
    CTRunRef run2 = (CTRunRef)CFArrayGetValueAtIndex(runs, 2);

    REPORTER_ASSERT(reporter, (CTRunGetStatus(run0) & kCTRunStatusRightToLeft) == 0);
    REPORTER_ASSERT(reporter, (CTRunGetStatus(run1) & kCTRunStatusRightToLeft) != 0);
    REPORTER_ASSERT(reporter, (CTRunGetStatus(run2) & kCTRunStatusRightToLeft) == 0);

    REPORTER_ASSERT(reporter, CTRunGetGlyphCount(run0) > 0);
    REPORTER_ASSERT(reporter, CTRunGetGlyphCount(run1) > 0);
    REPORTER_ASSERT(reporter, CTRunGetGlyphCount(run2) > 0);
}

DEF_TEST(CoreTextPipeline_LineShaper_SinglePassShaping, reporter) {
    ParagraphStyle paragraphStyle;
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    TextStyle style;
    style.setFontSize(16.0f);
    builder.pushStyle(style);
    const char* text = "Hello CoreText World";
    size_t textLen = strlen(text);
    builder.addText(text, textLen);
    builder.pop();

    auto paragraph = builder.Build();
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

    CoreTextLineShaper shaper(impl);
    REPORTER_ASSERT(reporter, shaper.shape());
    REPORTER_ASSERT(reporter, shaper.unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);

    auto& run = impl->runs()[0];
    REPORTER_ASSERT(reporter, run.textRange() == TextRange(0, textLen));
    REPORTER_ASSERT(reporter, run.leftToRight());
    REPORTER_ASSERT(reporter, run.bidiLevel() == 0);
    REPORTER_ASSERT(reporter, run.script() == SkSetFourByteTag('L', 'a', 't', 'n'));
    REPORTER_ASSERT(reporter, run.size() > 0);
    REPORTER_ASSERT(reporter, run.glyphs().size() == run.size());
    REPORTER_ASSERT(reporter, run.positions().size() == run.size() + 1);
    REPORTER_ASSERT(reporter, run.offsets().size() == run.size() + 1);
    REPORTER_ASSERT(reporter, run.clusterIndexes().size() == run.size() + 1);
    REPORTER_ASSERT(reporter, run.advance().fX > 0.0f);
    REPORTER_ASSERT(reporter, run.positions()[0].fX == 0.0f);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(run.positions()[run.size()].fX, run.advance().fX));
    REPORTER_ASSERT(reporter, run.font().getSize() == 16.0f);
    REPORTER_ASSERT(reporter, !run.isCursiveScript());
    REPORTER_ASSERT(reporter, !impl->resolvedFonts().empty());
    REPORTER_ASSERT(reporter, impl->resolvedFonts()[0].fTextStart == 0);

    impl->buildClusterTable();
    REPORTER_ASSERT(reporter, impl->clusters().size() > 1);
}

DEF_TEST(CoreTextPipeline_LineShaper_BiDiLogicalReordering, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Logical run order must be strictly monotonically increasing across directional transitions
    {
        ParagraphStyle ltrStyle;
        ltrStyle.setTextDirection(TextDirection::kLtr);
        ParagraphBuilderImpl builder(ltrStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(14.0f);
        builder.pushStyle(style);
        const char* bidiText = "Hello سلام World";
        builder.addText(bidiText, strlen(bidiText));
        builder.pop();

        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

        CoreTextLineShaper shaper(impl);
        REPORTER_ASSERT(reporter, shaper.shape());
        REPORTER_ASSERT(reporter, impl->runs().size() >= 3);

        for (size_t i = 1; i < impl->runs().size(); ++i) {
            REPORTER_ASSERT(reporter, impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
        }

        REPORTER_ASSERT(reporter, impl->runs()[0].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[0].bidiLevel() == 0);
        REPORTER_ASSERT(reporter, !impl->runs()[1].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[1].bidiLevel() == 1);
        REPORTER_ASSERT(reporter, impl->runs()[2].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[2].bidiLevel() == 0);
    }

    // Embedded LTR within an RTL paragraph requires nested even bidi level 2
    {
        ParagraphStyle rtlStyle;
        rtlStyle.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(rtlStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(14.0f);
        builder.pushStyle(style);
        const char* rtlText = "سلام Hello سلام";
        builder.addText(rtlText, strlen(rtlText));
        builder.pop();

        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

        CoreTextLineShaper shaper(impl);
        REPORTER_ASSERT(reporter, shaper.shape());
        REPORTER_ASSERT(reporter, impl->runs().size() == 5);

        for (size_t i = 1; i < impl->runs().size(); ++i) {
            REPORTER_ASSERT(reporter, impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
        }

        REPORTER_ASSERT(reporter, !impl->runs()[0].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[0].bidiLevel() == 1);
        REPORTER_ASSERT(reporter, !impl->runs()[1].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[1].bidiLevel() == 1);
        REPORTER_ASSERT(reporter, impl->runs()[2].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[2].bidiLevel() == 2);
        REPORTER_ASSERT(reporter, !impl->runs()[3].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[3].bidiLevel() == 1);
        REPORTER_ASSERT(reporter, !impl->runs()[4].leftToRight());
        REPORTER_ASSERT(reporter, impl->runs()[4].bidiLevel() == 1);
    }
}

DEF_TEST(CoreTextPipeline_LineShaper_ScriptDetection, reporter) {
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("Hello", 5) == SkSetFourByteTag('L', 'a', 't', 'n'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("Привет", 12) == SkSetFourByteTag('C', 'y', 'r', 'l'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("سلام", 8) == SkSetFourByteTag('A', 'r', 'a', 'b'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("שלום", 8) == SkSetFourByteTag('H', 'e', 'b', 'r'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xD7\x90", 2) == SkSetFourByteTag('H', 'e', 'b', 'r'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xDC\x90", 2) == SkSetFourByteTag('S', 'y', 'r', 'c'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xDF\x80", 2) == SkSetFourByteTag('N', 'k', 'o', 'o'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xE0\xA4\x85", 3) == SkSetFourByteTag('D', 'e', 'v', 'a'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xE0\xB8\x81", 3) == SkSetFourByteTag('T', 'h', 'a', 'i'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xE6\xBC\xA2", 3) == SkSetFourByteTag('H', 'a', 'n', 'i'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xE3\x81\x82", 3) == SkSetFourByteTag('H', 'i', 'r', 'a'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xE3\x82\xA2", 3) == SkSetFourByteTag('K', 'a', 'n', 'a'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript("\xED\x95\x9C", 3) == SkSetFourByteTag('H', 'a', 'n', 'g'));
    REPORTER_ASSERT(reporter, CoreTextLineShaper::detectScript(nullptr, 0) == SkSetFourByteTag('L', 'a', 't', 'n'));

    REPORTER_ASSERT(reporter, !CoreTextLineShaper::isLastResortFont(nullptr));

    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Cursive script detection drives run letter-spacing eligibility
    ParagraphStyle style;
    ParagraphBuilderImpl builder(style, fontCollection, get_unicode());
    TextStyle textStyle;
    textStyle.setFontSize(14.0f);
    builder.pushStyle(textStyle);
    builder.addText("سلام");
    builder.pop();

    auto paragraph = builder.Build();
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    impl->computeCodeUnitProperties();

    CoreTextLineShaper shaper(impl);
    REPORTER_ASSERT(reporter, shaper.shape());
    REPORTER_ASSERT(reporter, impl->runs().size() == 1);
    REPORTER_ASSERT(reporter, impl->runs()[0].isCursiveScript());
    REPORTER_ASSERT(reporter, impl->runs()[0].script() == SkSetFourByteTag('A', 'r', 'a', 'b'));
}

DEF_TEST(CoreTextPipeline_LineShaper_ClusterAtomicity, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Multi-byte UTF-8 Cyrillic cluster boundaries must stay byte-aligned
    {
        ParagraphStyle style;
        ParagraphBuilderImpl builder(style, fontCollection, get_unicode());
        TextStyle textStyle;
        textStyle.setFontSize(14.0f);
        builder.pushStyle(textStyle);
        const char* cyrText = "Кластер";
        size_t cyrLen = strlen(cyrText);
        builder.addText(cyrText, cyrLen);
        builder.pop();

        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

        CoreTextLineShaper shaper(impl);
        REPORTER_ASSERT(reporter, shaper.shape());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);

        auto& run = impl->runs()[0];
        REPORTER_ASSERT(reporter, run.textRange().width() == cyrLen);
        REPORTER_ASSERT(reporter, run.clusterIndexes()[0] == 0);
        REPORTER_ASSERT(reporter, run.clusterIndexes()[run.size()] == cyrLen);

        for (size_t k = 0; k <= run.size(); ++k) {
            REPORTER_ASSERT(reporter, run.clusterIndexes()[k] % 2 == 0);
            REPORTER_ASSERT(reporter, run.clusterIndexes()[k] <= cyrLen);
        }

        size_t lastCharEnd = run.textRange().start;
        size_t clusterCount = 0;
        run.iterateThroughClustersInTextOrder([&](size_t glyphStart,
                                                  size_t glyphEnd,
                                                  size_t charStart,
                                                  size_t charEnd,
                                                  SkScalar width,
                                                  SkScalar height) {
            REPORTER_ASSERT(reporter, charStart == lastCharEnd);
            REPORTER_ASSERT(reporter, charEnd > charStart);
            REPORTER_ASSERT(reporter, glyphEnd > glyphStart);
            lastCharEnd = charEnd;
            ++clusterCount;
        });
        REPORTER_ASSERT(reporter, lastCharEnd == run.textRange().end);
        REPORTER_ASSERT(reporter, clusterCount == 7);

        impl->buildClusterTable();
        REPORTER_ASSERT(reporter, impl->clusters().size() == 8);
        for (size_t i = 0; i < cyrLen; ++i) {
            REPORTER_ASSERT(reporter, impl->clusterIndex(i) == i / 2);
        }
    }

    // RTL cluster traversal in logical text order
    {
        ParagraphStyle rtlStyle;
        rtlStyle.setTextDirection(TextDirection::kRtl);
        ParagraphBuilderImpl builder(rtlStyle, fontCollection, get_unicode());
        TextStyle textStyle;
        textStyle.setFontSize(14.0f);
        builder.pushStyle(textStyle);
        const char* rtlText = "اختبار";
        size_t rtlLen = strlen(rtlText);
        builder.addText(rtlText, rtlLen);
        builder.pop();

        auto paragraph = builder.Build();
        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

        CoreTextLineShaper shaper(impl);
        REPORTER_ASSERT(reporter, shaper.shape());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);

        auto& run = impl->runs()[0];
        REPORTER_ASSERT(reporter, !run.leftToRight());

        size_t lastCharEnd = run.textRange().start;
        size_t clusterCount = 0;
        run.iterateThroughClustersInTextOrder([&](size_t glyphStart,
                                                  size_t glyphEnd,
                                                  size_t charStart,
                                                  size_t charEnd,
                                                  SkScalar width,
                                                  SkScalar height) {
            REPORTER_ASSERT(reporter, charStart == lastCharEnd);
            REPORTER_ASSERT(reporter, charEnd > charStart);
            lastCharEnd = charEnd;
            ++clusterCount;
        });
        REPORTER_ASSERT(reporter, lastCharEnd == run.textRange().end);
        REPORTER_ASSERT(reporter, clusterCount > 0);
    }
}

DEF_TEST(CoreTextPipeline_LineShaper_WithPlaceholders, reporter) {
    ParagraphStyle paragraphStyle;
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
    TextStyle style;
    style.setFontSize(14.0f);
    builder.pushStyle(style);
    builder.addText("Prefix ");
    PlaceholderStyle placeholderStyle(30.0f, 20.0f, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0.0f);
    builder.addPlaceholder(placeholderStyle);
    builder.addText(" Suffix");
    builder.pop();

    auto paragraph = builder.Build();
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->computeCodeUnitProperties());

    CoreTextLineShaper shaper(impl);
    REPORTER_ASSERT(reporter, shaper.shape());
    REPORTER_ASSERT(reporter, shaper.unresolvedGlyphs() == 0);

    REPORTER_ASSERT(reporter, impl->runs().size() == 3);

    auto& run0 = impl->runs()[0];
    auto& run1 = impl->runs()[1];
    auto& run2 = impl->runs()[2];

    REPORTER_ASSERT(reporter, !run0.isPlaceholder());
    REPORTER_ASSERT(reporter, run0.textRange() == TextRange(0, 7));
    REPORTER_ASSERT(reporter, run0.advance().fX > 0.0f);

    REPORTER_ASSERT(reporter, run1.isPlaceholder());
    REPORTER_ASSERT(reporter, run1.advance().fX == 30.0f);
    REPORTER_ASSERT(reporter, run1.positions()[0].fX == run0.advance().fX);
    REPORTER_ASSERT(reporter, run1.positions()[1].fX == run0.advance().fX + 30.0f);

    REPORTER_ASSERT(reporter, !run2.isPlaceholder());
    REPORTER_ASSERT(reporter, run2.advance().fX > 0.0f);
    REPORTER_ASSERT(reporter, run2.positions()[0].fX == run0.advance().fX + 30.0f);

    impl->buildClusterTable();
    REPORTER_ASSERT(reporter, impl->clusters().size() > 3);
}

DEF_TEST(CoreTextPipeline_Layout_LetterSpacingAndWordSpacing, reporter) {
    ParagraphStyle paragraphStyle;
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto createParagraph = [&](const char* text, SkScalar letterSpacing, SkScalar wordSpacing) {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(20.0f);
        style.setLetterSpacing(letterSpacing);
        style.setWordSpacing(wordSpacing);
        builder.pushStyle(style);
        builder.addText(text);
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);
        return paragraph;
    };

    auto pNormal = createParagraph("Hello World", 0.0f, 0.0f);
    auto pLetterSpaced = createParagraph("Hello World", 5.0f, 0.0f);
    auto pWordSpaced = createParagraph("Hello World", 0.0f, 10.0f);
    auto pCombined = createParagraph("Hello World", 5.0f, 10.0f);

    const SkScalar normalWidth = pNormal->getMaxIntrinsicWidth();
    const SkScalar letterSpacedWidth = pLetterSpaced->getMaxIntrinsicWidth();
    const SkScalar wordSpacedWidth = pWordSpaced->getMaxIntrinsicWidth();
    const SkScalar combinedWidth = pCombined->getMaxIntrinsicWidth();

    // "Hello World" is 11 codepoints; letter spacing 5.0 adds exactly 55.0 points
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(letterSpacedWidth - normalWidth, 55.0f, 0.05f));

    // Exactly 1 space cluster; word spacing 10.0 adds exactly 10.0 points
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(wordSpacedWidth - normalWidth, 10.0f, 0.05f));

    // Combined spacing adds 55.0 (letter) + 10.0 (word) = 65.0 points
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(combinedWidth - normalWidth, 65.0f, 0.05f));

    // Verify cluster halfLetterSpacing contract for Flutter caret geometry
    auto implLS = static_cast<ParagraphImpl*>(pLetterSpaced.get());
    for (const auto& cluster : implLS->clusters()) {
        if (cluster.runOrNull() && !cluster.run().isPlaceholder()) {
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(cluster.getHalfLetterSpacing(), 2.5f, 0.01f));
        }
    }

    // Multi-style span test with distinct letter spacings
    {
        ParagraphBuilderImpl multiBuilder(paragraphStyle, fontCollection, get_unicode());
        TextStyle s1;
        s1.setFontSize(20.0f);
        s1.setLetterSpacing(4.0f);
        multiBuilder.pushStyle(s1);
        multiBuilder.addText("Hello ");
        multiBuilder.pop();

        TextStyle s2;
        s2.setFontSize(20.0f);
        s2.setLetterSpacing(2.0f);
        multiBuilder.pushStyle(s2);
        multiBuilder.addText("World");
        multiBuilder.pop();

        auto pMulti = multiBuilder.Build();
        pMulti->layout(1000.0f);

        // "Hello " (6 chars * 4 = 24) + "World" (5 chars * 2 = 10) = 34.0 points extra
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pMulti->getMaxIntrinsicWidth() - normalWidth, 34.0f, 0.05f));
    }

    // Arabic cursive script test: letter spacing must be suppressed to preserve cursive shaping
    {
        const char* arabic = "سلام";
        auto pArabNormal = createParagraph(arabic, 0.0f, 0.0f);
        auto pArabSpaced = createParagraph(arabic, 50.0f, 0.0f);

        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(
            pArabNormal->getMaxIntrinsicWidth(), pArabSpaced->getMaxIntrinsicWidth(), 0.05f));
    }

    // Hit-testing and caret geometry contracts with letter spacing
    {
        auto boxes = pLetterSpaced->getRectsForRange(0, 5, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxes.empty());
        REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0.0f);
    }
}

DEF_TEST(CoreTextPipeline_AppleSystemFonts_OpticalAndTraits, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();

    // Default empty family resolves to system font
    {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(16.0f);
        builder.pushStyle(style);
        builder.addText("System Font Regular");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->runs().size() == 1);
        auto& run = impl->runs()[0];
        REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(run.font().getTypeface()));
    }

    // Bold and italic traits on system font
    {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle boldStyle;
        boldStyle.setFontSize(16.0f);
        boldStyle.setFontStyle(SkFontStyle::Bold());
        builder.pushStyle(boldStyle);
        builder.addText("Bold");
        builder.pop();

        TextStyle italicStyle;
        italicStyle.setFontSize(16.0f);
        italicStyle.setFontStyle(SkFontStyle::Italic());
        builder.pushStyle(italicStyle);
        builder.addText(" Italic");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);
    }

    // Named system font aliases
    for (const char* alias : {".AppleSystemUIFont", "SF Pro", "system-ui", "-apple-system"}) {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontFamilies({SkString(alias)});
        style.setFontSize(14.0f);
        builder.pushStyle(style);
        builder.addText("Apple Alias Test");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(500.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);
    }
}

DEF_TEST(CoreTextPipeline_AppleSystemFonts_MultiScriptFallbackChain, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Single pass shapes mixed Latin + CJK + Emoji + Arabic via CoreText platform fallback
    {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(16.0f);
        builder.pushStyle(style);
        const char* mixedText = "Hello 世界 🎉 مرحبا";
        builder.addText(mixedText);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->runs().size() >= 4);

        // Logical text order invariant across script runs
        for (size_t i = 1; i < impl->runs().size(); ++i) {
            REPORTER_ASSERT(reporter, impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
        }

        bool foundLatin = false;
        bool foundCJK = false;
        bool foundEmoji = false;
        bool foundArabic = false;

        for (const auto& run : impl->runs()) {
            REPORTER_ASSERT(reporter, run.size() > 0);
            REPORTER_ASSERT(reporter, run.advance().fX > 0.0f);
            REPORTER_ASSERT(reporter, run.font().getTypeface() != nullptr);

            SkString family;
            run.font().getTypeface()->getFamilyName(&family);
            SkString psName;
            run.font().getTypeface()->getPostScriptName(&psName);

            REPORTER_ASSERT(reporter, !family.contains("LastResort"));
            REPORTER_ASSERT(reporter, !psName.contains("LastResort"));

            if (family.contains("Emoji") || psName.contains("Emoji")) {
                foundEmoji = true;
                REPORTER_ASSERT(reporter, run.leftToRight());
            } else if (run.script() == SkSetFourByteTag('L', 'a', 't', 'n')) {
                foundLatin = true;
                REPORTER_ASSERT(reporter, run.leftToRight());
            } else if (run.script() == SkSetFourByteTag('H', 'a', 'n', 'i')) {
                foundCJK = true;
                REPORTER_ASSERT(reporter, run.leftToRight());
                REPORTER_ASSERT(reporter, family.contains("PingFang") || family.contains("Hiragino") ||
                                          family.contains("Songti") || family.contains("Heiti") ||
                                          psName.contains("PingFang") || psName.contains("Hiragino"));
            } else if (run.script() == SkSetFourByteTag('A', 'r', 'a', 'b')) {
                foundArabic = true;
                REPORTER_ASSERT(reporter, !run.leftToRight());
                REPORTER_ASSERT(reporter, run.isCursiveScript());
            }
        }

        REPORTER_ASSERT(reporter, foundLatin);
        REPORTER_ASSERT(reporter, foundCJK);
        REPORTER_ASSERT(reporter, foundEmoji);
        REPORTER_ASSERT(reporter, foundArabic);

        // Caret and selection hit-testing geometry across script boundaries in UTF-16 code units
        auto boxesLatin = paragraph->getRectsForRange(0, 5, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesLatin.empty());
        if (!boxesLatin.empty()) {
            REPORTER_ASSERT(reporter, boxesLatin[0].rect.width() > 0.0f);
        }

        auto boxesCJK = paragraph->getRectsForRange(6, 8, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesCJK.empty());
        if (!boxesCJK.empty()) {
            REPORTER_ASSERT(reporter, boxesCJK[0].rect.width() > 0.0f);
        }

        auto boxesEmoji = paragraph->getRectsForRange(9, 11, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesEmoji.empty());
        if (!boxesEmoji.empty()) {
            REPORTER_ASSERT(reporter, boxesEmoji[0].rect.width() > 0.0f);
        }

        auto boxesArabic = paragraph->getRectsForRange(12, 17, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesArabic.empty());
        if (!boxesArabic.empty()) {
            REPORTER_ASSERT(reporter, boxesArabic[0].rect.width() > 0.0f);
        }

        // Verification of canvas text blob paint execution across fallback fonts
        SkBitmap bitmap;
        bitmap.allocN32Pixels(500, 100);
        SkCanvas canvas(bitmap);
        canvas.clear(SK_ColorWHITE);
        paragraph->paint(&canvas, 0, 0);
    }

    // Explicit cascade list ordering via kCTFontCascadeListAttribute
    {
        ParagraphStyle paragraphStyle;
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode());
        TextStyle style;
        style.setFontSize(16.0f);
        style.setFontFamilies({SkString("Courier"), SkString("Apple Color Emoji"), SkString("PingFang SC")});
        builder.pushStyle(style);
        builder.addText("Courier: 世界 🎉");
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0.0f);

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        REPORTER_ASSERT(reporter, impl->runs().size() >= 3);

        auto& runCourier = impl->runs()[0];
        SkString family0;
        runCourier.font().getTypeface()->getFamilyName(&family0);
        REPORTER_ASSERT(reporter, family0.contains("Courier"));
    }
}

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

