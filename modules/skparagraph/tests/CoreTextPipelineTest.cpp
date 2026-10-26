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
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skunicode/include/SkUnicode.h"
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_apple.h"
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

using namespace skia::textlayout;

namespace {
// Each engine gets the SkUnicode its clients ship with: Apple's for CoreText, ICU for HarfBuzz.
// A build lacking it yields null, which ParagraphBuilder::Build rejects instead of silently
// laying out with another implementation
sk_sp<SkUnicode> get_unicode(const ParagraphStyle& style) {
#if defined(SK_UNICODE_APPLE_IMPLEMENTATION)
    if (style.getTextShapingEngine() == TextShapingEngine::kCoreText) {
        return SkUnicodes::Apple::Make();
    }
#endif
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
    return SkUnicodes::ICU::Make();
#else
    return nullptr;
#endif
}

sk_sp<FontCollection> test_font_collection() {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    return fontCollection;
}

// The expectations name Apple system faces; the portable font manager (--nonativeFonts) has
// none of them, so such tests skip instead of measuring substitutes
bool has_font_families(std::initializer_list<const char*> families) {
    for (const char* family : families) {
        if (!ToolUtils::TestFontMgr()->matchFamilyStyle(family, SkFontStyle())) {
            return false;
        }
    }
    return true;
}
}  // namespace

#if !defined(SK_PARAGRAPH_NATIVE_CORETEXT)

DEF_TEST(CoreTextPipeline_DisabledFallbackToHarfBuzz, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    TextStyle style;
    style.setFontFamilies({SkString("Roboto")});
    style.setFontSize(16.0f);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText("Fallback to HarfBuzz when CoreText is disabled");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(500.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, !impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() > 0);
}

#else  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#include "include/core/SkFontArguments.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"
#include "modules/skparagraph/src/CoreTextLineShaper.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUTF.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <optional>
#include <string.h>
#include <vector>

DEF_TEST(CoreTextPipeline_IndexMap_AsciiAndMultibyte, reporter) {
    CoreTextIndexMap emptyMap("", 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf16Index(0) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf8Index(0) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf16Index(10) == 0);
    REPORTER_ASSERT(reporter, emptyMap.toUtf8Index(10) == 0);

    // Validate fast path for single-byte encoding and range roundtrips
    const char* ascii = "Hello, world!";
    size_t asciiLen = strlen(ascii);
    CoreTextIndexMap asciiMap(ascii, asciiLen);
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
    for (int i = 0; i < kEmojiCount; ++i) {
        REPORTER_ASSERT(reporter, heapMap.toUtf16Index(i * 4) == static_cast<size_t>(i * 2));
        REPORTER_ASSERT(reporter, heapMap.toUtf8Index(i * 2) == static_cast<size_t>(i * 4));
    }
}

DEF_TEST(CoreTextPipeline_FontRegistry_DescriptorCacheAndFallback, reporter) {
    // Apple system font detection and descriptor caching
    CoreTextFontRegistry& registry = CoreTextFontRegistry::Instance();

    REPORTER_ASSERT(
            reporter,
            CoreTextFontRegistry::isAppleSystemFont(static_cast<const SkTypeface*>(nullptr)));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".AppleSystemUIFont"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".AppleSystemUIFontRounded"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".AppleSystemUIFontSerif"));
    REPORTER_ASSERT(
            reporter, CoreTextFontRegistry::isAppleSystemFont(".AppleSystemUIFontMonospaced"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont("CupertinoSystemText"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont("CupertinoSystemDisplay"));
    REPORTER_ASSERT(reporter, CoreTextFontRegistry::isAppleSystemFont(".SF NS"));
    REPORTER_ASSERT(reporter, !CoreTextFontRegistry::isAppleSystemFont("SF Pro"));
    REPORTER_ASSERT(reporter, !CoreTextFontRegistry::isAppleSystemFont("system-ui"));
    REPORTER_ASSERT(reporter, !CoreTextFontRegistry::isAppleSystemFont("-apple-system"));
    REPORTER_ASSERT(reporter, !CoreTextFontRegistry::isAppleSystemFont("CourierNewCustom"));

    auto sysFont = registry.createFont(nullptr, SkFontStyle(), 18.0f);
    REPORTER_ASSERT(reporter, sysFont != nullptr);
    REPORTER_ASSERT(reporter, CTFontGetSize(sysFont.get()) == 18.0f);

    auto tf = ToolUtils::DefaultPortableTypeface();
    if (tf) {
        auto desc1 = registry.getDescriptor(tf.get());
        REPORTER_ASSERT(reporter, desc1 != nullptr);
        auto desc2 = registry.getDescriptor(tf.get());
        REPORTER_ASSERT(reporter, desc2 != nullptr);
    }
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_BaseDirectionBiDi, reporter) {
    // CoreText paragraph style must encode LTR and RTL base embedding direction
    const char* ltrText = "Hello world";
    size_t ltrLen = strlen(ltrText);
    CoreTextAttrStringBuilder ltrBuilder(
            ltrText, ltrLen, TextDirection::kLtr, test_font_collection());
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
    CoreTextAttrStringBuilder rtlBuilder(
            rtlText, rtlLen, TextDirection::kRtl, test_font_collection());
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
    CoreTextAttrStringBuilder builder(text, textLen, TextDirection::kLtr, test_font_collection());
    TextStyle style;
    // No family binds .AppleSystemUIFont, which has both features; CTFontCopyAttribute drops
    // settings the bound font does not support
    style.setFontFamilies({});
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

DEF_TEST(CoreTextPipeline_AttrStringBuilder_LetterSpacingTracking, reporter) {
    // Letter spacing uses native CoreText tracking attribute (CTTracking) on the full range
    const CFStringRef kTrackingKey = CFSTR("CTTracking");

    const char* latin = "ABCDEF";
    size_t latinLen = strlen(latin);
    CoreTextAttrStringBuilder latinBuilder(
            latin, latinLen, TextDirection::kLtr, test_font_collection());
    TextStyle latinStyle;
    latinStyle.setLetterSpacing(3.5f);
    latinBuilder.addStyle(TextRange(0, latinLen), latinStyle);
    auto latinAttr = latinBuilder.build();
    REPORTER_ASSERT(reporter, latinAttr != nullptr);

    CFNumberRef trackNum = (CFNumberRef)CFAttributedStringGetAttribute(
        latinAttr.get(), 0, kTrackingKey, nullptr);
    REPORTER_ASSERT(reporter, trackNum != nullptr);
    CGFloat trackVal = 0.0;
    CFNumberGetValue(trackNum, kCFNumberCGFloatType, &trackVal);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(trackVal, 3.5f));

    const char* arabic = "سلام";
    size_t arabicLen = strlen(arabic);
    CoreTextAttrStringBuilder arabicBuilder(
            arabic, arabicLen, TextDirection::kRtl, test_font_collection());
    TextStyle arabicStyle;
    arabicStyle.setLetterSpacing(3.5f);
    arabicBuilder.addStyle(TextRange(0, arabicLen), arabicStyle);
    auto arabicAttr = arabicBuilder.build();
    REPORTER_ASSERT(reporter, arabicAttr != nullptr);

    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        arabicAttr.get(), 0, kTrackingKey, nullptr) == nullptr);

    const char* mixed = "Hi سلام Hi";
    size_t mixedLen = strlen(mixed);
    CoreTextAttrStringBuilder mixedBuilder(
            mixed, mixedLen, TextDirection::kLtr, test_font_collection());
    TextStyle mixedStyle;
    mixedStyle.setLetterSpacing(3.5f);
    mixedBuilder.addStyle(TextRange(0, mixedLen), mixedStyle);
    auto mixedAttr = mixedBuilder.build();
    REPORTER_ASSERT(reporter, mixedAttr != nullptr);

    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        mixedAttr.get(), 0, kTrackingKey, nullptr) != nullptr);

    size_t u16ArabicIdx = mixedBuilder.indexMap().toUtf16Index(3);
    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        mixedAttr.get(), u16ArabicIdx, kTrackingKey, nullptr) == nullptr);
    size_t u16TrailingLatinIdx = mixedBuilder.indexMap().toUtf16Index(mixedLen - 1);
    REPORTER_ASSERT(reporter, CFAttributedStringGetAttribute(
        mixedAttr.get(), u16TrailingLatinIdx, kTrackingKey, nullptr) != nullptr);
}

DEF_TEST(CoreTextPipeline_AttrStringBuilder_NoTrackingByDefault, reporter) {
    // Invariant: without explicit letter spacing, no synthetic tracking is applied
    const CFStringRef kTrackingKey = CFSTR("CTTracking");
    const char* text = "No tracking check";
    size_t textLen = strlen(text);
    CoreTextAttrStringBuilder builder(text, textLen, TextDirection::kLtr, test_font_collection());
    TextStyle style;
    style.setFontSize(16.0f);
    builder.addStyle(TextRange(0, textLen), style);
    auto attrStr = builder.build();
    REPORTER_ASSERT(reporter, attrStr != nullptr);

    CFNumberRef trackingNum = (CFNumberRef)CFAttributedStringGetAttribute(
        attrStr.get(), 0, kTrackingKey, nullptr);
    REPORTER_ASSERT(reporter, trackingNum == nullptr);
}


DEF_TEST(CoreTextPipeline_LineShapingAndBiDiLevels, reporter) {
    // CTLine separates runs by direction with correct kCTRunStatusRightToLeft flags
    const char* bidiText = "Hello سلام World";
    size_t bidiLen = strlen(bidiText);
    CoreTextAttrStringBuilder builder(
            bidiText, bidiLen, TextDirection::kLtr, test_font_collection());
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

    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
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
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(run.positions()[run.size()].fX, run.advance().fX));
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
        ltrStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(ltrStyle, fontCollection, get_unicode(ltrStyle));
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
            REPORTER_ASSERT(
                    reporter,
                    impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
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
        rtlStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(rtlStyle, fontCollection, get_unicode(rtlStyle));
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
            REPORTER_ASSERT(
                    reporter,
                    impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
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
    auto scriptAt = [](const char* text, size_t utf8Index) {
        CoreTextAttrStringBuilder builder(
                text, strlen(text), TextDirection::kLtr, test_font_collection());
        return builder.scriptAt(utf8Index);
    };
    REPORTER_ASSERT(reporter, scriptAt("Hello", 0) == SkSetFourByteTag('L', 'a', 't', 'n'));
    REPORTER_ASSERT(reporter, scriptAt("Привет", 0) == SkSetFourByteTag('C', 'y', 'r', 'l'));
    REPORTER_ASSERT(reporter, scriptAt("سلام", 0) == SkSetFourByteTag('A', 'r', 'a', 'b'));
    REPORTER_ASSERT(reporter, scriptAt("שלום", 0) == SkSetFourByteTag('H', 'e', 'b', 'r'));
    REPORTER_ASSERT(reporter, scriptAt("\xD7\x90", 0) == SkSetFourByteTag('H', 'e', 'b', 'r'));
    REPORTER_ASSERT(reporter, scriptAt("\xDC\x90", 0) == SkSetFourByteTag('S', 'y', 'r', 'c'));
    REPORTER_ASSERT(reporter, scriptAt("\xDF\x80", 0) == SkSetFourByteTag('N', 'k', 'o', 'o'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xE0\xA4\x85", 0) == SkSetFourByteTag('D', 'e', 'v', 'a'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xE0\xB8\x81", 0) == SkSetFourByteTag('T', 'h', 'a', 'i'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xE6\xBC\xA2", 0) == SkSetFourByteTag('H', 'a', 'n', 'i'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xE3\x81\x82", 0) == SkSetFourByteTag('H', 'i', 'r', 'a'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xE3\x82\xA2", 0) == SkSetFourByteTag('K', 'a', 'n', 'a'));
    REPORTER_ASSERT(reporter,
                    scriptAt("\xED\x95\x9C", 0) == SkSetFourByteTag('H', 'a', 'n', 'g'));

    // Common code points join the preceding script, or the following one when leading
    REPORTER_ASSERT(reporter, scriptAt("Hi \xD8\xB3", 2) == SkSetFourByteTag('L', 'a', 't', 'n'));
    REPORTER_ASSERT(reporter, scriptAt("Hi \xD8\xB3", 3) == SkSetFourByteTag('A', 'r', 'a', 'b'));
    REPORTER_ASSERT(reporter, scriptAt("\xD8\xB3 1", 3) == SkSetFourByteTag('A', 'r', 'a', 'b'));
    REPORTER_ASSERT(reporter, scriptAt("1 \xD8\xB3", 0) == SkSetFourByteTag('A', 'r', 'a', 'b'));
    REPORTER_ASSERT(reporter, scriptAt("123", 0) == SkSetFourByteTag('L', 'a', 't', 'n'));

    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Cursive script detection drives run letter-spacing eligibility
    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, get_unicode(style));
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
        style.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(style, fontCollection, get_unicode(style));
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
        rtlStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(rtlStyle, fontCollection, get_unicode(rtlStyle));
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

    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    TextStyle style;
    style.setFontSize(14.0f);
    builder.pushStyle(style);
    builder.addText("Prefix ");
    PlaceholderStyle placeholderStyle(
            30.0f, 20.0f, PlaceholderAlignment::kBaseline, TextBaseline::kAlphabetic, 0.0f);
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
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto createParagraph = [&](const char* text, SkScalar letterSpacing, SkScalar wordSpacing) {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        TextStyle style;
        // Pin Helvetica so baseline at 0.0f has no .SFNS optical trak table
        style.setFontFamilies({SkString("Helvetica")});
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
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(cluster.getHalfLetterSpacing(), 2.5f, 0.01f));
        }
    }

    // Multi-style span test with distinct letter spacings
    {
        ParagraphBuilderImpl multiBuilder(
                paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        TextStyle s1;
        s1.setFontFamilies({SkString("Helvetica")});
        s1.setFontSize(20.0f);
        s1.setLetterSpacing(4.0f);
        multiBuilder.pushStyle(s1);
        multiBuilder.addText("Hello ");
        multiBuilder.pop();

        TextStyle s2;
        s2.setFontFamilies({SkString("Helvetica")});
        s2.setFontSize(20.0f);
        s2.setLetterSpacing(2.0f);
        multiBuilder.pushStyle(s2);
        multiBuilder.addText("World");
        multiBuilder.pop();

        auto pMulti = multiBuilder.Build();
        pMulti->layout(1000.0f);

        // "Hello " (6 chars * 4 = 24) + "World" (5 chars * 2 = 10) = 34.0 points extra
        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(pMulti->getMaxIntrinsicWidth() - normalWidth, 34.0f, 0.05f));
    }

    // Arabic cursive script stays untracked, as on the HarfBuzz path
    {
        const char* arabic = "سلام";
        auto pArabNormal = createParagraph(arabic, 0.0f, 0.0f);
        auto pArabSpaced = createParagraph(arabic, 50.0f, 0.0f);

        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(pArabSpaced->getMaxIntrinsicWidth(),
                                            pArabNormal->getMaxIntrinsicWidth(),
                                            0.05f));
    }

    // Hit-testing and caret geometry contracts with letter spacing
    {
        auto boxes = pLetterSpaced->getRectsForRange(
                0, 5, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxes.empty());
        REPORTER_ASSERT(reporter, boxes[0].rect.width() > 0.0f);
    }
}

DEF_TEST(CoreTextPipeline_AppleSystemFonts_OpticalAndTraits, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();

    // Default empty family resolves to system font
    {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
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
        REPORTER_ASSERT(reporter,
                        CoreTextFontRegistry::isAppleSystemFont(run.font().getTypeface()));
    }

    // Bold and italic traits on system font
    {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
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
        paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
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
    if (!has_font_families({"Courier", "Apple Color Emoji", "PingFang SC"})) {
        return;
    }
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Single pass shapes mixed Latin + CJK + Emoji + Arabic via CoreText platform fallback
    {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
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
            REPORTER_ASSERT(
                    reporter,
                    impl->runs()[i].textRange().start >= impl->runs()[i - 1].textRange().end);
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
                // The space after the ideographs inherits their script but stays in the
                // primary font
                const char* first = mixedText + run.textRange().start;
                if (SkUTF::NextUTF8(&first, mixedText + run.textRange().end) == ' ') {
                    continue;
                }
                foundCJK = true;
                REPORTER_ASSERT(reporter, run.leftToRight());
                REPORTER_ASSERT(reporter,
                                family.contains("PingFang") || family.contains("Hiragino") ||
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
        auto boxesLatin =
                paragraph->getRectsForRange(0, 5, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesLatin.empty());
        if (!boxesLatin.empty()) {
            REPORTER_ASSERT(reporter, boxesLatin[0].rect.width() > 0.0f);
        }

        auto boxesCJK =
                paragraph->getRectsForRange(6, 8, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesCJK.empty());
        if (!boxesCJK.empty()) {
            REPORTER_ASSERT(reporter, boxesCJK[0].rect.width() > 0.0f);
        }

        auto boxesEmoji =
                paragraph->getRectsForRange(9, 11, RectHeightStyle::kMax, RectWidthStyle::kTight);
        REPORTER_ASSERT(reporter, !boxesEmoji.empty());
        if (!boxesEmoji.empty()) {
            REPORTER_ASSERT(reporter, boxesEmoji[0].rect.width() > 0.0f);
        }

        auto boxesArabic =
                paragraph->getRectsForRange(12, 17, RectHeightStyle::kMax, RectWidthStyle::kTight);
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
        paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        TextStyle style;
        style.setFontSize(16.0f);
        style.setFontFamilies(
                {SkString("Courier"), SkString("Apple Color Emoji"), SkString("PingFang SC")});
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

DEF_TEST(CoreTextPipeline_DisableFontFallback, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->disableFontFallback();
    REPORTER_ASSERT(reporter, !fontCollection->fontFallbackEnabled());

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    TextStyle style;
    style.setFontFamilies({SkString("Courier")});
    style.setFontSize(16.0f);

    {
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        builder.pushStyle(style);
        const char* mixedText = "Hello 世界 🎉 مرحبا";
        builder.addText(mixedText);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() > 0);
        auto unresolvedSet = paragraph->unresolvedCodepoints();
        REPORTER_ASSERT(reporter, !unresolvedSet.empty());

        auto impl = static_cast<ParagraphImpl*>(paragraph.get());
        for (const auto& run : impl->runs()) {
            if (run.glyphs().empty()) {
                continue;
            }
            if (run.glyphs()[0] == 0) {
                for (size_t i = 0; i < run.size(); ++i) {
                    REPORTER_ASSERT(reporter, run.glyphs()[i] == 0);
                }
            }
        }
    }

    {
        ParagraphBuilderImpl builder1(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        builder1.pushStyle(style);
        builder1.addText("世界");
        builder1.pop();
        auto p1 = builder1.Build();
        p1->layout(1000.0f);

        ParagraphBuilderImpl builder2(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        builder2.pushStyle(style);
        builder2.addText("世\u200D界\u200B");
        builder2.pop();
        auto p2 = builder2.Build();
        p2->layout(1000.0f);

        REPORTER_ASSERT(
                reporter,
                SkScalarNearlyEqual(p1->getMaxIntrinsicWidth(), p2->getMaxIntrinsicWidth()));
        REPORTER_ASSERT(reporter, p2->unresolvedGlyphs() == 2);
        auto unresolvedP2 = p2->unresolvedCodepoints();
        REPORTER_ASSERT(reporter, unresolvedP2.size() == 2);
        REPORTER_ASSERT(reporter, unresolvedP2.count(0x4E16) == 1);
        REPORTER_ASSERT(reporter, unresolvedP2.count(0x754C) == 1);
        REPORTER_ASSERT(reporter, unresolvedP2.count(0x200D) == 0);
        REPORTER_ASSERT(reporter, unresolvedP2.count(0x200B) == 0);
    }

    {
        fontCollection->enableFontFallback();
        fontCollection->getParagraphCache()->reset();
        REPORTER_ASSERT(reporter, fontCollection->fontFallbackEnabled());

        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        builder.pushStyle(style);
        const char* mixedText = "Hello 世界 🎉 مرحبا";
        builder.addText(mixedText);
        builder.pop();

        auto paragraph = builder.Build();
        paragraph->layout(1000.0f);

        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, paragraph->unresolvedCodepoints().empty());
    }
}

DEF_TEST(CoreTextPipeline_EngineSwitchLetterSpacing, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto testEngine = [&](TextShapingEngine engine, bool expectedCoreText) {
        ParagraphStyle styleNoSpacing;
        styleNoSpacing.setTextShapingEngine(engine);
        TextStyle textStyleNoSpacing;
        textStyleNoSpacing.setFontFamilies({SkString("Roboto")});
        textStyleNoSpacing.setFontSize(16.0f);

        ParagraphBuilderImpl b1(styleNoSpacing, fontCollection, get_unicode(styleNoSpacing));
        b1.pushStyle(textStyleNoSpacing);
        b1.addText("Letter Spacing Test");
        b1.pop();
        auto p1 = b1.Build();
        p1->layout(1000.0f);

        auto impl1 = static_cast<ParagraphImpl*>(p1.get());
        REPORTER_ASSERT(reporter, impl1->isCoreTextShaped() == expectedCoreText);

        ParagraphStyle styleSpacing;
        styleSpacing.setTextShapingEngine(engine);
        TextStyle textStyleSpacing;
        textStyleSpacing.setFontFamilies({SkString("Roboto")});
        textStyleSpacing.setFontSize(16.0f);
        textStyleSpacing.setLetterSpacing(4.0f);

        ParagraphBuilderImpl b2(styleSpacing, fontCollection, get_unicode(styleSpacing));
        b2.pushStyle(textStyleSpacing);
        b2.addText("Letter Spacing Test");
        b2.pop();
        auto p2 = b2.Build();
        p2->layout(1000.0f);

        auto impl2 = static_cast<ParagraphImpl*>(p2.get());
        REPORTER_ASSERT(reporter, impl2->isCoreTextShaped() == expectedCoreText);
        REPORTER_ASSERT(reporter, p2->getMaxIntrinsicWidth() > p1->getMaxIntrinsicWidth());
    };

    testEngine(TextShapingEngine::kCoreText, true);
    // A HarfBuzz-free build aborts on kDefault, so only the CoreText half can run there
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
    testEngine(TextShapingEngine::kDefault, false);
#endif
}

// Keeping the two engines apart in the cache needs both engines, and kDefault needs HarfBuzz
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
DEF_TEST(CoreTextPipeline_CacheKeyEngineSeparation, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    ParagraphCache cache;
    cache.turnOn(true);

    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Roboto")});
    textStyle.setFontSize(16.0f);

    auto makeParagraph = [&](TextShapingEngine engine) {
        ParagraphStyle paragraphStyle;
        paragraphStyle.setTextShapingEngine(engine);
        ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
        builder.pushStyle(textStyle);
        builder.addText("Paragraph Cache Shaping Engine Test");
        builder.pop();
        auto paragraph = builder.Build();
        paragraph->layout(500.0f);
        return paragraph;
    };

    auto pCT1 = makeParagraph(TextShapingEngine::kCoreText);
    auto implCT1 = static_cast<ParagraphImpl*>(pCT1.get());
    REPORTER_ASSERT(reporter, implCT1->isCoreTextShaped());
    REPORTER_ASSERT(reporter, cache.updateParagraph(implCT1));
    REPORTER_ASSERT(reporter, cache.count() == 1);

    auto pHB1 = makeParagraph(TextShapingEngine::kDefault);
    auto implHB1 = static_cast<ParagraphImpl*>(pHB1.get());
    REPORTER_ASSERT(reporter, !implHB1->isCoreTextShaped());
    REPORTER_ASSERT(reporter, !cache.findParagraph(implHB1));

    REPORTER_ASSERT(reporter, cache.updateParagraph(implHB1));
    REPORTER_ASSERT(reporter, cache.count() == 2);

    ParagraphStyle ctQueryStyle;
    ctQueryStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl bCT(ctQueryStyle, fontCollection, get_unicode(ctQueryStyle));
    bCT.pushStyle(textStyle);
    bCT.addText("Paragraph Cache Shaping Engine Test");
    bCT.pop();
    auto pCTQuery = bCT.Build();
    auto implCTQuery = static_cast<ParagraphImpl*>(pCTQuery.get());
    REPORTER_ASSERT(reporter, !implCTQuery->isCoreTextShaped());
    REPORTER_ASSERT(reporter, cache.findParagraph(implCTQuery));
    REPORTER_ASSERT(reporter, implCTQuery->isCoreTextShaped());

    ParagraphStyle hbQueryStyle;
    hbQueryStyle.setTextShapingEngine(TextShapingEngine::kDefault);
    ParagraphBuilderImpl bHB(hbQueryStyle, fontCollection, get_unicode(hbQueryStyle));
    bHB.pushStyle(textStyle);
    bHB.addText("Paragraph Cache Shaping Engine Test");
    bHB.pop();
    auto pHBQuery = bHB.Build();
    auto implHBQuery = static_cast<ParagraphImpl*>(pHBQuery.get());
    REPORTER_ASSERT(reporter, !implHBQuery->isCoreTextShaped());
    REPORTER_ASSERT(reporter, cache.findParagraph(implHBQuery));
    REPORTER_ASSERT(reporter, !implHBQuery->isCoreTextShaped());
}
#endif  // !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)

namespace {
using Coordinate = SkFontArguments::VariationPosition::Coordinate;

// Lays out "abcabcabc" at 48pt in one family, optionally pinning a single variation axis.
// The sample is restricted to glyphs that fonts/Variable.ttf actually maps
std::unique_ptr<Paragraph> layout_axis_sample(const sk_sp<FontCollection>& fontCollection,
                                              TextShapingEngine engine,
                                              const char* family,
                                              std::optional<Coordinate> coord) {
    TextStyle style;
    style.setFontFamilies({SkString(family)});
    style.setFontSize(48.0f);
    if (coord) {
        SkFontArguments args;
        args.setVariationDesignPosition({&*coord, 1});
        style.setFontArguments(args);
    }
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(engine);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText("abcabcabc");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}

const SkTypeface* first_run_typeface(Paragraph* paragraph) {
    SkSpan<Run> runs = static_cast<ParagraphImpl*>(paragraph)->runs();
    return runs.empty() ? nullptr : runs[0].font().getTypeface();
}

// SkTypeface_Mac surfaces CTFontCopyVariation here, so the run typeface reports the exact
// position CoreText shaped with; axes left unset report the font's default value
std::optional<float> axis_value(const SkTypeface* typeface, SkFourByteTag axis) {
    constexpr int kMaxAxes = 16;
    Coordinate coords[kMaxAxes];
    const int count = typeface ? typeface->getVariationDesignPosition(coords) : -1;
    if (count < 0 || count > kMaxAxes) {
        return std::nullopt;
    }
    for (int i = 0; i < count; ++i) {
        if (coords[i].axis == axis) {
            return coords[i].value;
        }
    }
    return std::nullopt;
}

bool axis_near(const SkTypeface* typeface, SkFourByteTag axis, float expected) {
    std::optional<float> value = axis_value(typeface, axis);
    return value && SkScalarNearlyEqual(*value, expected, 0.5f);
}
}  // namespace

DEF_TEST(CoreTextPipeline_FontArguments_VariableFontAxes, reporter) {
    sk_sp<SkTypeface> variable = ToolUtils::CreateTypefaceFromResource("fonts/Variable.ttf");
    if (!variable) {
        // Only CoreText-backed font managers load resource data; the portable one cannot
        return;
    }
    auto provider = sk_make_sp<TypefaceFontProvider>();
    provider->registerTypeface(variable, SkString("Variable"));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(provider);
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    auto layout = [&](TextShapingEngine engine, std::optional<Coordinate> coord) {
        return layout_axis_sample(fontCollection, engine, "Variable", coord);
    };

    // TextStyle font arguments must reach the CTFont: wdth moves advances in this font
    // (wght does not), so the three layouts can only agree when the axis is dropped
    auto regular = layout(TextShapingEngine::kCoreText, std::nullopt);
    auto narrow = layout(TextShapingEngine::kCoreText, Coordinate{Coordinate::wdth, 50.0f});
    auto wide = layout(TextShapingEngine::kCoreText, Coordinate{Coordinate::wdth, 200.0f});
    for (Paragraph* paragraph : {regular.get(), narrow.get(), wide.get()}) {
        REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph)->isCoreTextShaped());
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    }
    const SkScalar regularWidth = regular->getMaxIntrinsicWidth();
    const SkScalar narrowWidth = narrow->getMaxIntrinsicWidth();
    const SkScalar wideWidth = wide->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, narrowWidth < regularWidth - 10.0f,
                    "wdth=50 width %g vs default %g", narrowWidth, regularWidth);
    REPORTER_ASSERT(reporter, wideWidth > regularWidth + 10.0f,
                    "wdth=200 width %g vs default %g", wideWidth, regularWidth);
    const SkTypeface* narrowTypeface = first_run_typeface(narrow.get());
    REPORTER_ASSERT(reporter, axis_near(narrowTypeface, Coordinate::wdth, 50.0f),
                    "run typeface reports wdth %g",
                    axis_value(narrowTypeface, Coordinate::wdth).value_or(-1.0f));

    // Both engines take advances from CoreText for the same varied face, so they must agree
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
    auto hbNarrow = layout(TextShapingEngine::kDefault, Coordinate{Coordinate::wdth, 50.0f});
    const SkScalar hbNarrowWidth = hbNarrow->getMaxIntrinsicWidth();
    REPORTER_ASSERT(reporter, !static_cast<ParagraphImpl*>(hbNarrow.get())->isCoreTextShaped());
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(hbNarrowWidth, narrowWidth, 1.0f),
                    "HarfBuzz wdth=50 width %g vs CoreText %g", hbNarrowWidth, narrowWidth);
#endif
}

DEF_TEST(CoreTextPipeline_FontArguments_SystemFontWeightAxis, reporter) {
    if (!has_font_families({".AppleSystemUIFont"})) {
        return;
    }
    // Flutter's default family on Apple platforms. wght must come from the font arguments,
    // while opsz keeps tracking the point size: pinning it (as SkCTFontCreateExactCopy does on
    // the HarfBuzz path) would freeze the optical size the system font picks per size
    auto check = [&](const sk_sp<FontCollection>& fontCollection) {
        auto layout = [&](std::optional<Coordinate> coord) {
            return layout_axis_sample(fontCollection, TextShapingEngine::kCoreText,
                                      ".AppleSystemUIFont", coord);
        };
        auto regular = layout(std::nullopt);
        auto black = layout(Coordinate{Coordinate::wght, 900.0f});
        auto thin = layout(Coordinate{Coordinate::wght, 100.0f});
        for (Paragraph* paragraph : {regular.get(), black.get(), thin.get()}) {
            REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph)->isCoreTextShaped());
            REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        }
        const SkScalar regularWidth = regular->getMaxIntrinsicWidth();
        const SkScalar blackWidth = black->getMaxIntrinsicWidth();
        const SkScalar thinWidth = thin->getMaxIntrinsicWidth();
        REPORTER_ASSERT(reporter, blackWidth > regularWidth + 20.0f,
                        "wght=900 width %g vs regular %g", blackWidth, regularWidth);
        REPORTER_ASSERT(reporter, thinWidth < regularWidth - 5.0f,
                        "wght=100 width %g vs regular %g", thinWidth, regularWidth);

        const SkTypeface* regularTypeface = first_run_typeface(regular.get());
        const SkTypeface* blackTypeface = first_run_typeface(black.get());
        REPORTER_ASSERT(reporter, axis_near(blackTypeface, Coordinate::wght, 900.0f),
                        "wght=900 run typeface reports wght %g",
                        axis_value(blackTypeface, Coordinate::wght).value_or(-1.0f));
        REPORTER_ASSERT(reporter, axis_near(regularTypeface, Coordinate::opsz, 48.0f),
                        "regular run typeface reports opsz %g",
                        axis_value(regularTypeface, Coordinate::opsz).value_or(-1.0f));
        REPORTER_ASSERT(reporter, axis_near(blackTypeface, Coordinate::opsz, 48.0f),
                        "wght=900 run typeface reports opsz %g",
                        axis_value(blackTypeface, Coordinate::opsz).value_or(-1.0f));
    };

    // An empty collection binds the system font through CoreText alone; a CoreText-backed
    // default manager resolves the same family to an SkTypeface_Mac first
    check(sk_make_sp<FontCollection>());
    auto withDefaultManager = sk_make_sp<FontCollection>();
    withDefaultManager->setDefaultFontManager(ToolUtils::TestFontMgr());
    check(withDefaultManager);
}

namespace {
// Flutter's default family on Apple platforms carrying the BCP-47 tag Flutter copies from
// Locale.toLanguageTag() into TextStyle::setLocale
TextStyle system_font_style(const char* locale) {
    TextStyle style;
    style.setFontFamilies({SkString(".AppleSystemUIFont")});
    style.setFontSize(16.0f);
    style.setLocale(SkString(locale));
    return style;
}

// Lays out U+9AA8 U+76F4 U+5203 in a single span through CoreText
std::unique_ptr<Paragraph> layout_locale_sample(const sk_sp<FontCollection>& fontCollection,
                                                const char* locale) {
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(system_font_style(locale));
    builder.addText("\xE9\xAA\xA8\xE7\x9B\xB4\xE5\x88\x83");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}

// Family of the typeface CoreText bound to the run; empty when the run does not exist so a
// failed contrast reports what was shaped instead of indexing past the run array
SkString run_family(Paragraph* paragraph, size_t index) {
    SkSpan<Run> runs = static_cast<ParagraphImpl*>(paragraph)->runs();
    SkString family;
    if (index < runs.size() && runs[index].font().getTypeface()) {
        runs[index].font().getTypeface()->getFamilyName(&family);
    }
    return family;
}
}  // namespace

DEF_TEST(CoreTextPipeline_Locale_LanguageAttributeDrivesCJKFallback, reporter) {
    // An empty collection leaves every fallback decision to CoreText, which is where the
    // span locale has to arrive as kCTLanguageAttributeName
    auto fontCollection = sk_make_sp<FontCollection>();
    auto ja = layout_locale_sample(fontCollection, "ja");
    auto jaJP = layout_locale_sample(fontCollection, "ja-JP");
    auto ko = layout_locale_sample(fontCollection, "ko");
    auto zhHant = layout_locale_sample(fontCollection, "zh-Hant");
    auto zhHans = layout_locale_sample(fontCollection, "zh-Hans");

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(system_font_style("ja"));
    builder.addText("\xE9\xAA\xA8");
    builder.pop();
    builder.pushStyle(system_font_style("zh-Hant"));
    builder.addText("\xE9\xAA\xA8");
    builder.pop();
    auto mixed = builder.Build();
    mixed->layout(1000.0f);

    for (Paragraph* paragraph :
         {ja.get(), jaJP.get(), ko.get(), zhHant.get(), zhHans.get(), mixed.get()}) {
        auto impl = static_cast<ParagraphImpl*>(paragraph);
        REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, !impl->runs().empty());
    }

    // Han unification: U+9AA8 U+76F4 U+5203 are the same code points in ja, zh-Hans, zh-Hant
    // and ko, so only the span locale can tell CoreText which regional font to fall back to.
    // Contrasts stay between explicit tags: the no-locale pick follows the machine's languages
    const SkString jaFamily = run_family(ja.get(), 0);
    const SkString jaJPFamily = run_family(jaJP.get(), 0);
    const SkString koFamily = run_family(ko.get(), 0);
    const SkString zhHantFamily = run_family(zhHant.get(), 0);
    const SkString zhHansFamily = run_family(zhHans.get(), 0);
    REPORTER_ASSERT(reporter, jaFamily.contains("Hiragino"),
                    "locale ja shaped with '%s'", jaFamily.c_str());
    REPORTER_ASSERT(reporter, jaJPFamily.contains("Hiragino"),
                    "locale ja-JP shaped with '%s'", jaJPFamily.c_str());
    REPORTER_ASSERT(reporter, koFamily.contains("Apple SD Gothic"),
                    "locale ko shaped with '%s'", koFamily.c_str());
    REPORTER_ASSERT(reporter, zhHantFamily.contains("PingFang") && zhHantFamily.contains("TC"),
                    "locale zh-Hant shaped with '%s'", zhHantFamily.c_str());
    REPORTER_ASSERT(reporter, !zhHansFamily.contains("Hiragino"),
                    "locale zh-Hans shaped with '%s'", zhHansFamily.c_str());

    // The language attribute must cover the span range only: a paragraph-wide tag or one
    // leaking out of the first span would shape both ideographs with the Japanese font
    const size_t mixedRuns = static_cast<ParagraphImpl*>(mixed.get())->runs().size();
    const SkString mixedJaFamily = run_family(mixed.get(), 0);
    const SkString mixedZhHantFamily = run_family(mixed.get(), 1);
    REPORTER_ASSERT(reporter, mixedRuns >= 2,
                    "ja + zh-Hant spans produced %zu run(s)", mixedRuns);
    REPORTER_ASSERT(reporter, mixedJaFamily.contains("Hiragino"),
                    "ja span shaped with '%s'", mixedJaFamily.c_str());
    REPORTER_ASSERT(reporter, mixedZhHantFamily.contains("TC"),
                    "zh-Hant span shaped with '%s'", mixedZhHantFamily.c_str());
}

namespace {
sk_sp<FontCollection> make_roboto_collection(const sk_sp<SkTypeface>& regular) {
    auto provider = sk_make_sp<TypefaceFontProvider>();
    provider->registerTypeface(regular, SkString("Roboto"));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(provider);
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    return fontCollection;
}

// The synthetic style contract is parity with OneLineShaper, which needs HarfBuzz
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
// Lays out "Synthetic" in one span so the first run carries the whole style request
std::unique_ptr<Paragraph> layout_styled_sample(const sk_sp<FontCollection>& fontCollection,
                                                TextShapingEngine engine,
                                                const char* family,
                                                SkFontStyle fontStyle,
                                                bool fakeMissingFontStyles) {
    TextStyle style;
    style.setFontFamilies({SkString(family)});
    style.setFontSize(24.0f);
    style.setFontStyle(fontStyle);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(engine);
    paragraphStyle.setFakeMissingFontStyles(fakeMissingFontStyles);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText("Synthetic");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}
#endif

// Font bound to the first run; nullptr when nothing was shaped so the caller reports the
// empty paragraph instead of indexing past the run array
const SkFont* first_run_font(Paragraph* paragraph) {
    SkSpan<Run> runs = static_cast<ParagraphImpl*>(paragraph)->runs();
    return runs.empty() ? nullptr : &runs[0].font();
}
}  // namespace

// HarfBuzz is the oracle every assertion below compares CoreText against
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
DEF_TEST(CoreTextPipeline_SyntheticStyles_ParityWithHarfBuzz, reporter) {
    sk_sp<SkTypeface> regular = ToolUtils::CreateTypefaceFromResource("fonts/Roboto-Regular.ttf");
    if (!regular) {
        // Only CoreText-backed font managers load resource data; the portable one cannot
        return;
    }
    const SkFontStyle boldItalic(SkFontStyle::kBold_Weight, SkFontStyle::kNormal_Width,
                                 SkFontStyle::kItalic_Slant);
    REPORTER_ASSERT(reporter, ParagraphStyle().fakeMissingFontStyles());

    auto fontCollection = make_roboto_collection(regular);
    auto ct = layout_styled_sample(fontCollection, TextShapingEngine::kCoreText, "Roboto",
                                   boldItalic, true);
    auto hb = layout_styled_sample(
            fontCollection, TextShapingEngine::kDefault, "Roboto", boldItalic, true);
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(ct.get())->isCoreTextShaped());
    REPORTER_ASSERT(reporter, !static_cast<ParagraphImpl*>(hb.get())->isCoreTextShaped());
    for (Paragraph* paragraph : {ct.get(), hb.get()}) {
        REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
        REPORTER_ASSERT(reporter, first_run_font(paragraph) != nullptr);
    }
    const SkFont* ctFont = first_run_font(ct.get());
    const SkFont* hbFont = first_run_font(hb.get());
    if (!ctFont || !hbFont) {
        return;
    }

    // Only a Regular face is registered, so both engines bind an upright sub-semibold typeface
    // and the bold italic request can only be met synthetically; a failure here means the
    // resource changed, not the pipeline
    for (const SkFont* font : {ctFont, hbFont}) {
        const SkFontStyle bound = font->getTypeface()->fontStyle();
        REPORTER_ASSERT(reporter, bound.weight() < SkFontStyle::kSemiBold_Weight,
                        "Roboto run typeface weight %d", bound.weight());
        REPORTER_ASSERT(reporter, bound.slant() != SkFontStyle::kItalic_Slant,
                        "Roboto run typeface slant %d", static_cast<int>(bound.slant()));
    }

    // OneLineShaper is the oracle for fakeMissingFontStyles; CoreText must leave the run SkFont
    // in the same state, and both sides are pinned absolutely so a regression in one engine
    // cannot hide behind the other
    REPORTER_ASSERT(reporter, hbFont->isEmbolden(), "HarfBuzz run not emboldened");
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(hbFont->getSkewX(), -0.25f),
                    "HarfBuzz run skewX %g", hbFont->getSkewX());
    REPORTER_ASSERT(reporter, ctFont->isEmbolden(), "CoreText run not emboldened");
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ctFont->getSkewX(), -0.25f),
                    "CoreText run skewX %g", ctFont->getSkewX());
    REPORTER_ASSERT(reporter, ctFont->isEmbolden() == hbFont->isEmbolden(),
                    "embolden CoreText %d vs HarfBuzz %d",
                    ctFont->isEmbolden(), hbFont->isEmbolden());
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ctFont->getSkewX(), hbFont->getSkewX()),
                    "skewX CoreText %g vs HarfBuzz %g", ctFont->getSkewX(), hbFont->getSkewX());

    // Sharing fontCollection also verifies ParagraphCache keys on fakeMissingFontStyles instead
    // of replaying the emboldened/skewed runs from the flag-on entries above
    for (TextShapingEngine engine : {TextShapingEngine::kCoreText, TextShapingEngine::kDefault}) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        auto plain = layout_styled_sample(fontCollection, engine, "Roboto", boldItalic, false);
        const SkFont* plainFont = first_run_font(plain.get());
        REPORTER_ASSERT(reporter,
                        static_cast<ParagraphImpl*>(plain.get())->isCoreTextShaped() == coreText);
        REPORTER_ASSERT(reporter, plainFont != nullptr);
        if (plainFont) {
            REPORTER_ASSERT(reporter, !plainFont->isEmbolden(),
                            "engine %d: fakeMissingFontStyles=false still emboldened the run",
                            static_cast<int>(engine));
            REPORTER_ASSERT(reporter, plainFont->getSkewX() == 0,
                            "engine %d: fakeMissingFontStyles=false still skewed the run by %g",
                            static_cast<int>(engine), plainFont->getSkewX());
        }
    }
    REPORTER_ASSERT(reporter, fontCollection->getParagraphCache()->count() == 4,
                    "Expected 4 cache entries (2 engines x 2 fakeMissingFontStyles); got %d",
                    fontCollection->getParagraphCache()->count());

    // CoreText binds a real bold system face for a bold request, so synthesis must compare
    // against the bound face and stay off; keying on the requested weight alone fails here
    auto systemBold = layout_styled_sample(sk_make_sp<FontCollection>(),
                                           TextShapingEngine::kCoreText, ".AppleSystemUIFont",
                                           SkFontStyle::Bold(), true);
    const SkFont* systemFont = first_run_font(systemBold.get());
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(systemBold.get())->isCoreTextShaped());
    REPORTER_ASSERT(reporter, systemBold->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, systemFont != nullptr);
    if (systemFont) {
        const int boundWeight = systemFont->getTypeface()->fontStyle().weight();
        REPORTER_ASSERT(reporter, boundWeight >= SkFontStyle::kSemiBold_Weight,
                        "system font bound weight %d for a bold request", boundWeight);
        REPORTER_ASSERT(reporter, !systemFont->isEmbolden(),
                        "real bold face (weight %d) was emboldened", boundWeight);
        REPORTER_ASSERT(reporter, systemFont->getSkewX() == 0,
                        "upright request skewed by %g", systemFont->getSkewX());
    }
}
#endif  // !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)

namespace {
// Records where TextLine hands each glyph blob to the canvas. The canvas places glyph k at
// (x, y) + the blob's own point k, so both halves are kept: their sum is the painted baseline
class BaselineShiftBlobRecorder : public ParagraphPainter {
public:
    static constexpr int kMaxBlobs = 4;
    struct Blob {
        SkScalar fX;
        SkScalar fY;
        uint32_t fGlyphCount;
        SkPoint fFirstGlyph;
    };

    void drawTextBlob(const sk_sp<SkTextBlob>& blob, SkScalar x, SkScalar y,
                      const SkPaintOrID&) override {
        // Every call is counted so an unexpected blob split is reported instead of overrunning
        // the fixed storage
        ++fTotal;
        if (fTotal > kMaxBlobs) {
            return;
        }
        Blob& record = fBlobs[fTotal - 1];
        record = {x, y, 0, {0, 0}};
        if (!blob) {
            return;
        }
        SkTextBlobRunIterator it(blob.get());
        if (!it.done() && it.positioning() == SkTextBlobRunIterator::kFull_Positioning) {
            record.fGlyphCount = it.glyphCount();
            if (record.fGlyphCount > 0) {
                record.fFirstGlyph = it.points()[0];
            }
        }
    }
    void drawTextShadow(const sk_sp<SkTextBlob>&, SkScalar, SkScalar, SkColor,
                        SkScalar) override {}
    void drawRect(const SkRect&, const SkPaintOrID&) override {}
    void drawFilledRect(const SkRect&, const DecorationStyle&) override {}
    void drawPath(const SkPath&, const DecorationStyle&) override {}
    void drawLine(SkScalar, SkScalar, SkScalar, SkScalar, const DecorationStyle&) override {}
    void clipRect(const SkRect&) override {}
    void translate(SkScalar, SkScalar) override {}
    void save() override {}
    void restore() override {}

    int fTotal = 0;
    Blob fBlobs[kMaxBlobs] = {};
};

// "X" on the line's own baseline followed by a smaller "2" shifted by shift; the size
// difference keeps the two spans in separate runs independently of the shift itself
std::unique_ptr<Paragraph> layout_shifted_pair(const sk_sp<FontCollection>& fontCollection,
                                               TextShapingEngine engine,
                                               SkScalar shift) {
    TextStyle plain;
    plain.setFontFamilies({SkString("Roboto")});
    plain.setFontSize(20.0f);
    TextStyle shifted = plain;
    shifted.setFontSize(12.0f);
    shifted.setBaselineShift(shift);

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(engine);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(plain);
    builder.addText("X");
    builder.pop();
    builder.pushStyle(shifted);
    builder.addText("2");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}

// Painted baseline of "2" minus painted baseline of "X" (Skia y grows downwards), or NaN when
// the paint did not produce the two single-glyph blobs the contrast relies on
SkScalar painted_shift_delta(skiatest::Reporter* reporter,
                             const sk_sp<FontCollection>& fontCollection,
                             TextShapingEngine engine,
                             SkScalar shift) {
    const bool coreText = engine == TextShapingEngine::kCoreText;
    const char* name = coreText ? "CoreText" : "HarfBuzz";
    auto paragraph = layout_shifted_pair(fontCollection, engine, shift);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped() == coreText);
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, paragraph->lineNumber() == 1);

    // Both shapers must hand the style shift to the run model: paint applies it from there
    SkSpan<Run> runs = impl->runs();
    REPORTER_ASSERT(reporter, runs.size() == 2, "%s shift %g: %zu run(s)", name, shift,
                    runs.size());
    for (const Run& run : runs) {
        const SkScalar expected = run.textRange().start == 0 ? 0.0f : shift;
        REPORTER_ASSERT(reporter, run.baselineShift() == expected,
                        "%s shift %g: run [%zu, %zu) carries baselineShift %g", name, shift,
                        run.textRange().start, run.textRange().end, run.baselineShift());
    }

    BaselineShiftBlobRecorder recorder;
    paragraph->paint(&recorder, 0.0f, 0.0f);
    REPORTER_ASSERT(reporter, recorder.fTotal == 2, "%s shift %g: %d blob(s) painted", name,
                    shift, recorder.fTotal);
    if (recorder.fTotal != 2) {
        return SK_ScalarNaN;
    }
    const auto& a = recorder.fBlobs[0];
    const auto& b = recorder.fBlobs[1];
    // LTR line: whichever blob lands further left is "X"
    const bool aIsLeft = a.fX + a.fFirstGlyph.fX < b.fX + b.fFirstGlyph.fX;
    const auto& plain = aIsLeft ? a : b;
    const auto& shifted = aIsLeft ? b : a;
    REPORTER_ASSERT(reporter, plain.fGlyphCount == 1 && shifted.fGlyphCount == 1,
                    "%s shift %g: blobs carry %u and %u glyph(s)", name, shift,
                    plain.fGlyphCount, shifted.fGlyphCount);
    return (shifted.fY + shifted.fFirstGlyph.fY) - (plain.fY + plain.fFirstGlyph.fY);
}
}  // namespace

DEF_TEST(CoreTextPipeline_BaselineShift_PaintParityWithHarfBuzz, reporter) {
    sk_sp<SkTypeface> roboto = ToolUtils::CreateTypefaceFromResource("fonts/Roboto-Regular.ttf");
    if (!roboto) {
        // Only CoreText-backed font managers load resource data; the portable one cannot
        return;
    }
    auto provider = sk_make_sp<TypefaceFontProvider>();
    provider->registerTypeface(roboto, SkString("Roboto"));
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setAssetFontManager(provider);
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // TextLine::buildTextBlob is the single place that applies Run::fBaselineShift, so glyph
    // positions coming back from either shaper must sit on the unshifted baseline; a shaper
    // that pre-shifts them paints the span back onto the baseline it was meant to leave.
    // The floor(+0.5) rounding of the painted baseline is what the 0.51 tolerance absorbs
    for (SkScalar shift : {6.0f, -6.0f}) {
        const SkScalar ctDelta =
                painted_shift_delta(reporter, fontCollection, TextShapingEngine::kCoreText, shift);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ctDelta, shift, 0.51f),
                        "CoreText paints a %g baseline shift as %g", shift, ctDelta);
        // A HarfBuzz-free build aborts on kDefault, so the parity half needs HarfBuzz
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
        const SkScalar hbDelta =
                painted_shift_delta(reporter, fontCollection, TextShapingEngine::kDefault, shift);
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(hbDelta, shift, 0.51f),
                        "HarfBuzz paints a %g baseline shift as %g",
                        shift,
                        hbDelta);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(ctDelta, hbDelta, 0.51f),
                        "shift %g: CoreText delta %g vs HarfBuzz %g", shift, ctDelta, hbDelta);
#endif
    }
}

namespace {
// Lays out text in one span of family through CoreText; callers turn the paragraph cache
// off so every call is an independent typesetting pass
std::unique_ptr<Paragraph> layout_identity_sample(const sk_sp<FontCollection>& fontCollection,
                                                  const char* family,
                                                  const char* text) {
    TextStyle style;
    style.setFontFamilies({SkString(family)});
    style.setFontSize(20.0f);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText(text);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    return paragraph;
}

// Identity of the typeface bound to the first run; 0 when nothing was shaped
SkTypefaceID first_run_typeface_id(Paragraph* paragraph) {
    const SkFont* font = first_run_font(paragraph);
    return font && font->getTypeface() ? font->getTypeface()->uniqueID() : 0;
}
}  // namespace

DEF_TEST(CoreTextPipeline_TypefaceIdentity_StableAcrossLayouts, reporter) {
    sk_sp<SkTypeface> roboto = ToolUtils::CreateTypefaceFromResource("fonts/Roboto-Regular.ttf");
    if (!roboto) {
        // Only CoreText-backed font managers load resource data; the portable one cannot
        return;
    }
    auto fontCollection = make_roboto_collection(roboto);
    // A cache hit replays the first layout's SkFonts, which would fake the stability under test
    fontCollection->getParagraphCache()->turnOn(false);

    // Glyph strikes downstream are keyed by SkTypefaceID, so a typeface minted per typesetting
    // pass would miss them on every relayout; the data-backed face goes through the registry's
    // descriptor path and the system font through its per-size re-creation
    for (const char* family : {"Roboto", ".AppleSystemUIFont"}) {
        auto first = layout_identity_sample(fontCollection, family, "Identity one");
        auto second = layout_identity_sample(fontCollection, family, "Identity two");
        REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(first.get())->isCoreTextShaped());
        REPORTER_ASSERT(reporter, first->unresolvedGlyphs() == 0);
        const SkTypefaceID firstId = first_run_typeface_id(first.get());
        const SkTypefaceID secondId = first_run_typeface_id(second.get());
        REPORTER_ASSERT(reporter, firstId != 0 && firstId == secondId,
                        "%s: typeface %u in the first layout, %u in the second", family,
                        firstId, secondId);

        first->markDirty();
        first->layout(500.0f);
        const SkTypefaceID relaidId = first_run_typeface_id(first.get());
        REPORTER_ASSERT(reporter, relaidId == firstId,
                        "%s: typeface %u before relayout, %u after", family, firstId, relaidId);
    }

    // The asset font must actually be what CoreText bound, or the Roboto pass above only
    // measured the system fallback twice
    auto asset = layout_identity_sample(fontCollection, "Roboto", "Identity");
    REPORTER_ASSERT(reporter, run_family(asset.get(), 0).equals("Roboto"),
                    "Roboto resolved to '%s'", run_family(asset.get(), 0).c_str());
}

namespace {
// Width of text in the default family at 20pt; clusters receives the cluster count so the
// caller can tell letter spacing from a changed cluster table
SkScalar tracked_width(skiatest::Reporter* reporter,
                       const sk_sp<FontCollection>& fontCollection,
                       TextShapingEngine engine,
                       const char* text,
                       SkScalar letterSpacing,
                       size_t* clusters) {
    TextStyle style;
    // Pin Helvetica so 0.0f baseline has no .SFNS optical trak table
    style.setFontFamilies({SkString("Helvetica")});
    style.setFontSize(20.0f);
    style.setLetterSpacing(letterSpacing);
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(engine);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText(text);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(1000.0f);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter,
                    impl->isCoreTextShaped() == (engine == TextShapingEngine::kCoreText));
    REPORTER_ASSERT(reporter, paragraph->unresolvedGlyphs() == 0);
    *clusters = impl->clusters().size();
    return paragraph->getLongestLine();
}
}  // namespace

DEF_TEST(CoreTextPipeline_LetterSpacing_SkipsCursiveScriptsLikeHarfBuzz, reporter) {
    // The HarfBuzz half needs the test font manager to resolve Arabic by fallback
    if (!ToolUtils::TestFontMgr()->matchFamilyStyleCharacter(
                nullptr, SkFontStyle(), nullptr, 0, 0x0645)) {
        return;
    }
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());

    // Five letters each, so one tracking unit per code point is the full-tracking width
    const char* kArabic = "\xD9\x85\xD8\xB1\xD8\xAD\xD8\xA8\xD8\xA7";
    const char* kLatin = "Latin";
    const SkScalar kSpacing = 4.0f;
    const SkScalar kFullTracking = 5 * kSpacing;
    // A HarfBuzz-free build aborts on kDefault, so only the CoreText half can run there
    for (TextShapingEngine engine : {
#if !defined(SK_PARAGRAPH_DISABLE_HARFBUZZ)
                 TextShapingEngine::kDefault,
#endif
                 TextShapingEngine::kCoreText}) {
        const bool coreText = engine == TextShapingEngine::kCoreText;
        const char* name = coreText ? "CoreText" : "HarfBuzz";
        size_t plainClusters = 0;
        size_t trackedClusters = 0;
        const SkScalar latinDelta =
                tracked_width(reporter, fontCollection, engine, kLatin, kSpacing,
                              &trackedClusters) -
                tracked_width(reporter, fontCollection, engine, kLatin, 0.0f, &plainClusters);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(latinDelta, kFullTracking, 0.01f),
                        "%s: Latin grew by %g for letterSpacing %g", name, latinDelta, kSpacing);
        REPORTER_ASSERT(reporter, trackedClusters == plainClusters);

        // Both engines leave joining scripts alone (css-text-4 cursive tracking)
        const SkScalar expectedArabic = 0.0f;
        const SkScalar arabicDelta =
                tracked_width(reporter, fontCollection, engine, kArabic, kSpacing,
                              &trackedClusters) -
                tracked_width(reporter, fontCollection, engine, kArabic, 0.0f, &plainClusters);
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(arabicDelta, expectedArabic, 0.01f),
                        "%s: Arabic grew by %g for letterSpacing %g, expected %g", name,
                        arabicDelta, kSpacing, expectedArabic);
        REPORTER_ASSERT(reporter, trackedClusters == plainClusters,
                        "%s: %zu clusters tracked vs %zu plain", name, trackedClusters,
                        plainClusters);
    }
}

namespace {
struct CursiveSplit {
    SkScalar fCursiveWidth = 0;
    SkScalar fOtherWidth = 0;
    size_t fOtherCodepoints = 0;
    std::vector<SkGlyphID> fCursiveGlyphs;
};

// Splits the CoreText runs of text by Run::isCursiveScript after checking that flag against
// the run's letters; a run of spaces only inherits the script of its neighbors
CursiveSplit split_cursive_runs(skiatest::Reporter* reporter,
                                const sk_sp<FontCollection>& fontCollection,
                                const char* family,
                                const char* text,
                                SkScalar letterSpacing) {
    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    TextStyle style;
    style.setFontFamilies({SkString(family)});
    style.setFontSize(36.0f);
    style.setLetterSpacing(letterSpacing);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText(text);
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(10000.0f);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());

    CursiveSplit split;
    for (const Run& run : impl->runs()) {
        bool hasArabic = false;
        bool hasLatin = false;
        size_t codepoints = 0;
        const char* ptr = text + run.textRange().start;
        const char* end = text + run.textRange().end;
        while (ptr < end) {
            SkUnichar u = SkUTF::NextUTF8(&ptr, end);
            SkASSERT(u >= 0);
            hasArabic |= u >= 0x0600 && u <= 0x06FF;
            hasLatin |= (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z');
            ++codepoints;
        }
        REPORTER_ASSERT(reporter, !(hasArabic && hasLatin));
        REPORTER_ASSERT(reporter, run.isCursiveScript() == hasArabic || (!hasArabic && !hasLatin),
                        "%s: run [%zu, %zu) cursive=%d, Arabic=%d, Latin=%d", family,
                        run.textRange().start, run.textRange().end, run.isCursiveScript(),
                        hasArabic, hasLatin);
        if (run.isCursiveScript()) {
            split.fCursiveWidth += run.advance().fX;
            split.fCursiveGlyphs.insert(
                    split.fCursiveGlyphs.end(), run.glyphs().begin(), run.glyphs().end());
        } else {
            split.fOtherWidth += run.advance().fX;
            split.fOtherCodepoints += codepoints;
        }
    }
    return split;
}
}  // namespace

// css-text-4 forbids tracking cursive scripts; the HarfBuzz path skips them in
// ParagraphImpl::applySpacingAndBuildClusterTable, and CTTracking on SF Arabic inserts glyphs
// that break the joins, so CoreText must leave Arabic untouched while Latin keeps its spacing
DEF_TEST(CoreTextPipeline_LetterSpacing_CursiveArabicSystemFonts, reporter) {
    if (!has_font_families({".AppleSystemUIFont", "Geeza Pro"})) {
        return;
    }
    auto fontCollection = test_font_collection();
    const char* kArabic = "مرحبا بالعالم";
    const char* kMixed = "Hello مرحبا بالعالم";
    const SkScalar kSpacing = 10.0f;
    for (const char* family : {".AppleSystemUIFont", "Geeza Pro"}) {
        for (const char* text : {kArabic, kMixed}) {
            CursiveSplit plain = split_cursive_runs(reporter, fontCollection, family, text, 0);
            CursiveSplit spaced =
                    split_cursive_runs(reporter, fontCollection, family, text, kSpacing);
            REPORTER_ASSERT(reporter, !plain.fCursiveGlyphs.empty());
            REPORTER_ASSERT(reporter, spaced.fCursiveGlyphs == plain.fCursiveGlyphs,
                            "%s '%s': %zu Arabic glyphs plain, %zu spaced", family, text,
                            plain.fCursiveGlyphs.size(), spaced.fCursiveGlyphs.size());
            REPORTER_ASSERT(reporter,
                            SkScalarNearlyEqual(spaced.fCursiveWidth, plain.fCursiveWidth, 0.01f),
                            "%s '%s': Arabic width %g plain, %g spaced", family, text,
                            plain.fCursiveWidth, spaced.fCursiveWidth);
            REPORTER_ASSERT(reporter, spaced.fOtherCodepoints == plain.fOtherCodepoints);
            // Explicit spacing replaces the .SFNS auto trak (under 0.5pt per code point at
            // 36pt) instead of stacking on it, see ExplicitLetterSpacingReplacesAutoTrak
            const SkScalar otherDelta = spaced.fOtherWidth - plain.fOtherWidth;
            const SkScalar expectedOther = kSpacing * plain.fOtherCodepoints;
            REPORTER_ASSERT(reporter,
                            SkScalarAbs(otherDelta - expectedOther) <=
                                    0.5f * plain.fOtherCodepoints + 0.05f,
                            "%s '%s': Latin grew by %g, expected %g", family, text, otherDelta,
                            expectedOther);
        }
        CursiveSplit mixed = split_cursive_runs(reporter, fontCollection, family, kMixed, 0);
        REPORTER_ASSERT(reporter, mixed.fOtherCodepoints >= 5);
    }
}

DEF_TEST(CoreTextPipeline_ZeroFontSizeHasNoWidth, reporter) {
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, get_unicode(style));
    TextStyle textStyle;
    textStyle.setFontSize(0);
    builder.pushStyle(textStyle);
    builder.addText("abc");
    auto paragraph = builder.Build();
    paragraph->layout(500);
    REPORTER_ASSERT(reporter, paragraph->getMaxIntrinsicWidth() == 0,
                    "zero-size text is %g wide", paragraph->getMaxIntrinsicWidth());
    // Zero sizes stay on CoreText so a build without HarfBuzz lays them out as well
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->isCoreTextShaped());
}

DEF_TEST(CoreTextPipeline_EllipsisShapedByCoreText, reporter) {
    if (!has_font_families({"Helvetica"})) {
        return;
    }
    auto fontCollection = test_font_collection();
    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    style.setMaxLines(1);
    style.setEllipsis(u"\u2026");
    ParagraphBuilderImpl builder(style, fontCollection, get_unicode(style));
    TextStyle textStyle;
    textStyle.setFontFamilies({SkString("Helvetica")});
    textStyle.setFontSize(20);
    builder.pushStyle(textStyle);
    builder.addText("Ellipsis must survive a build without HarfBuzz");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(150);
    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, paragraph->didExceedMaxLines());
    REPORTER_ASSERT(reporter, impl->lines().size() == 1);
    if (impl->lines().size() != 1) {
        return;
    }
    const Run* ellipsis = impl->lines()[0].ellipsis();
    REPORTER_ASSERT(reporter, ellipsis && ellipsis->size() == 1 && ellipsis->isResolved());
    REPORTER_ASSERT(reporter,
                    paragraph->getLongestLine() <= 150,
                    "ellipsized line is %g wide",
                    paragraph->getLongestLine());
}

DEF_TEST(CoreTextPipeline_InvalidUtf8MatchesIndexMap, reporter) {
    // A truncated three-byte sequence followed by ASCII
    const char text[] = "\xE2\x82" "a";
    const size_t bytes = sizeof(text) - 1;
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    CoreTextAttrStringBuilder attrBuilder(text, bytes, TextDirection::kLtr, fontCollection);
    attrBuilder.addStyle(TextRange(0, bytes), TextStyle());
    auto attr = attrBuilder.build();
    REPORTER_ASSERT(reporter, attr != nullptr);
    REPORTER_ASSERT(reporter, SkToSizeT(CFAttributedStringGetLength(attr.get())) ==
                                      attrBuilder.indexMap().toUtf16Index(bytes));

    ParagraphStyle style;
    style.setTextShapingEngine(TextShapingEngine::kCoreText);
    ParagraphBuilderImpl builder(style, fontCollection, get_unicode(style));
    builder.pushStyle(TextStyle());
    builder.addText(text, bytes);
    auto paragraph = builder.Build();
    paragraph->layout(500);
    REPORTER_ASSERT(reporter, static_cast<ParagraphImpl*>(paragraph.get())->isCoreTextShaped());
}

namespace {
// Forwards to a real manager and counts every family or typeface query, so a test can prove a
// code path never consults it
class CountingFontMgr final : public SkFontMgr {
public:
    explicit CountingFontMgr(sk_sp<SkFontMgr> fontMgr) : fFontMgr(std::move(fontMgr)) {}
    int queries() const { return fQueries; }

protected:
    int onCountFamilies() const override {
        ++fQueries;
        return fFontMgr->countFamilies();
    }
    void onGetFamilyName(int index, SkString* familyName) const override {
        ++fQueries;
        fFontMgr->getFamilyName(index, familyName);
    }
    sk_sp<SkFontStyleSet> onCreateStyleSet(int index) const override {
        ++fQueries;
        return fFontMgr->createStyleSet(index);
    }
    sk_sp<SkFontStyleSet> onMatchFamily(const char familyName[]) const override {
        ++fQueries;
        return fFontMgr->matchFamily(familyName);
    }
    sk_sp<SkTypeface> onMatchFamilyStyle(const char familyName[],
                                         const SkFontStyle& style) const override {
        ++fQueries;
        return fFontMgr->matchFamilyStyle(familyName, style);
    }
    sk_sp<SkTypeface> onMatchFamilyStyleCharacter(const char familyName[],
                                                  const SkFontStyle& style,
                                                  const char* bcp47[],
                                                  int bcp47Count,
                                                  SkUnichar character) const override {
        ++fQueries;
        return fFontMgr->matchFamilyStyleCharacter(
                familyName, style, bcp47, bcp47Count, character);
    }
    sk_sp<SkTypeface> onMakeFromData(sk_sp<SkData> data, int ttcIndex) const override {
        ++fQueries;
        return fFontMgr->makeFromData(std::move(data), ttcIndex);
    }
    sk_sp<SkTypeface> onMakeFromStreamIndex(std::unique_ptr<SkStreamAsset> stream,
                                            int ttcIndex) const override {
        ++fQueries;
        return fFontMgr->makeFromStream(std::move(stream), ttcIndex);
    }
    sk_sp<SkTypeface> onMakeFromStreamArgs(std::unique_ptr<SkStreamAsset> stream,
                                           const SkFontArguments& args) const override {
        ++fQueries;
        return fFontMgr->makeFromStream(std::move(stream), args);
    }
    sk_sp<SkTypeface> onMakeFromFile(const char path[], int ttcIndex) const override {
        ++fQueries;
        return fFontMgr->makeFromFile(path, ttcIndex);
    }
    sk_sp<SkTypeface> onLegacyMakeTypeface(const char familyName[],
                                           SkFontStyle style) const override {
        ++fQueries;
        return fFontMgr->legacyMakeTypeface(familyName, style);
    }

private:
    sk_sp<SkFontMgr> fFontMgr;
    mutable int fQueries = 0;
};
}  // namespace

// System families, the cascade for CJK, Arabic and emoji, the strut, empty-line metrics and the
// ellipsis all resolve through CoreText, so the default manager is never asked anything
DEF_TEST(CoreTextPipeline_DefaultFontManagerIgnored, reporter) {
    if (!has_font_families({"Helvetica", "PingFang SC", "Apple Color Emoji"})) {
        return;
    }
    auto counting = sk_make_sp<CountingFontMgr>(ToolUtils::TestFontMgr());
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(counting);
    fontCollection->enableFontFallback();

    ParagraphStyle paragraphStyle;
    paragraphStyle.setTextShapingEngine(TextShapingEngine::kCoreText);
    paragraphStyle.setMaxLines(1);
    paragraphStyle.setEllipsis(u"\u2026");
    StrutStyle strut;
    strut.setStrutEnabled(true);
    strut.setFontFamilies({SkString("Helvetica")});
    strut.setFontSize(16.0f);
    paragraphStyle.setStrutStyle(strut);

    TextStyle style;
    style.setFontFamilies({SkString("Helvetica")});
    style.setFontSize(16.0f);
    ParagraphBuilderImpl builder(paragraphStyle, fontCollection, get_unicode(paragraphStyle));
    builder.pushStyle(style);
    builder.addText("Latin \xE4\xB8\xAD\xE6\x96\x87 \xD8\xB9\xD8\xB1\xD8\xA8\xD9\x8A "
                    "\xF0\x9F\x98\x80 and enough trailing text to need an ellipsis\n\n");
    builder.pop();
    auto paragraph = builder.Build();
    paragraph->layout(200.0f);

    auto impl = static_cast<ParagraphImpl*>(paragraph.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, paragraph->didExceedMaxLines());
    REPORTER_ASSERT(reporter, impl->unresolvedGlyphs() == 0);
    REPORTER_ASSERT(reporter, counting->queries() == 0, "queries=%d", counting->queries());
}

// CupertinoSystemText and CupertinoSystemDisplay must resolve in matchSystemFamily so that a
// non-empty fontFamilyFallback list does not skip them and land on the first fallback family
DEF_TEST(CoreTextPipeline_AppleSystemFonts_CupertinoAliases, reporter) {
    if (!has_font_families({"Helvetica"})) {
        return;
    }
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    CoreTextFontRegistry& registry = CoreTextFontRegistry::Instance();
    for (const char* alias : {"CupertinoSystemText", "CupertinoSystemDisplay"}) {
        std::vector<SkString> families = {SkString(alias), SkString("Helvetica")};
        auto faces = registry.findTypefaces(
                *fontCollection, families, SkFontStyle::Normal(), std::nullopt);
        REPORTER_ASSERT(reporter, faces.size() == 2, "alias=%s size=%zu", alias, faces.size());
        if (faces.size() == 2) {
            SkString firstPs;
            faces[0]->getPostScriptName(&firstPs);
            REPORTER_ASSERT(reporter,
                            firstPs.startsWith(".AppleSystemUIFont") || firstPs.startsWith(".SF"),
                            "alias=%s resolved to %s",
                            alias,
                            firstPs.c_str());
            SkString secondPs;
            faces[1]->getPostScriptName(&secondPs);
            REPORTER_ASSERT(reporter, secondPs.equals("Helvetica"));
        }
    }
}

// .AppleSystemUIFont{Rounded,Serif,Monospaced} select their UI design variants across both
// typeface matching and size-dependent CTFont instantiation (including bold weight traits)
DEF_TEST(CoreTextPipeline_AppleSystemFonts_DesignSuffixes, reporter) {
    if (!has_font_families({"Helvetica"})) {
        return;
    }
    auto fontCollection = sk_make_sp<FontCollection>();
    fontCollection->setDefaultFontManager(ToolUtils::TestFontMgr());
    fontCollection->enableFontFallback();

    CoreTextFontRegistry& registry = CoreTextFontRegistry::Instance();
    const struct {
        const char* fFamily;
        const char* fExpectedRegularPs;
        const char* fExpectedBoldPs;
    } cases[] = {
            {".AppleSystemUIFontRounded", "Rounded-Regular", "Rounded-Bold"},
            {".AppleSystemUIFontSerif", "NewYork-Regular", "NewYork-Bold"},
            {".AppleSystemUIFontMonospaced", "Monospaced-Regular", "Monospaced-Bold"},
    };

    for (const auto& c : cases) {
        std::vector<SkString> families = {SkString(c.fFamily), SkString("Helvetica")};
        auto regFaces = registry.findTypefaces(
                *fontCollection, families, SkFontStyle::Normal(), std::nullopt);
        auto boldFaces = registry.findTypefaces(
                *fontCollection, families, SkFontStyle::Bold(), std::nullopt);
        REPORTER_ASSERT(reporter, regFaces.size() == 2 && boldFaces.size() == 2);
        if (regFaces.size() != 2 || boldFaces.size() != 2) {
            continue;
        }
        SkString regPs;
        regFaces[0]->getPostScriptName(&regPs);
        REPORTER_ASSERT(reporter,
                        regPs.contains(c.fExpectedRegularPs),
                        "family=%s got=%s",
                        c.fFamily,
                        regPs.c_str());

        SkUniqueCFRef<CTFontRef> ctBold =
                registry.createFont(boldFaces[0].get(), SkFontStyle::Bold(), 24.0f);
        REPORTER_ASSERT(reporter, ctBold != nullptr);
        if (ctBold) {
            SkUniqueCFRef<CFStringRef> cfPs(CTFontCopyPostScriptName(ctBold.get()));
            char buf[256] = {};
            if (cfPs) {
                CFStringGetCString(cfPs.get(), buf, sizeof(buf), kCFStringEncodingUTF8);
            }
            REPORTER_ASSERT(reporter,
                            strstr(buf, c.fExpectedBoldPs) != nullptr,
                            "family=%s ctBold=%s",
                            c.fFamily,
                            buf);
        }
    }
}

// Explicit letterSpacing (e.g. CupertinoTextThemeData's -0.41pt at 17pt) must suppress .SFNS's
// built-in trak table via NSCTFontUnscaledTrackingAttribute=0 rather than stacking on top of it
DEF_TEST(CoreTextPipeline_AppleSystemFonts_ExplicitLetterSpacingReplacesAutoTrak, reporter) {
    if (!has_font_families({".AppleSystemUIFont"})) {
        return;
    }
    auto fc = test_font_collection();
    const char* sample = "The quick brown fox jumps over the lazy dog";

    auto measure = [&](SkScalar letterSpacing) -> SkScalar {
        ParagraphStyle ps;
        ps.setTextShapingEngine(TextShapingEngine::kCoreText);
        TextStyle ts;
        ts.setFontFamilies({SkString("CupertinoSystemText")});
        ts.setFontSize(17.0f);
        ts.setLetterSpacing(letterSpacing);
        ParagraphBuilderImpl b(ps, fc, get_unicode(ps));
        b.pushStyle(ts);
        b.addText(sample);
        b.pop();
        auto p = b.Build();
        p->layout(2000.0f);
        return p->getMaxIntrinsicWidth();
    };

    // At 17pt, .SFNS's built-in trak is -0.432 pt/ch; CupertinoTextThemeData specifies
    // -0.41 pt/ch. Replacing auto trak with -0.41 keeps the 43-char line within ~1.5pt of
    // auto-trak (letterSpacing=0), whereas double-tracking would shrink it by ~17.6pt
    SkScalar wAutoTrak = measure(0.0f);
    SkScalar wExplicitCupertino = measure(-0.41f);
    REPORTER_ASSERT(reporter,
                    SkScalarAbs(wExplicitCupertino - wAutoTrak) < 2.5f,
                    "auto=%f explicit(-0.41)=%f",
                    wAutoTrak,
                    wExplicitCupertino);
}

// Empty fontFamilies with an unresolved glyph (e.g. U+10FFFF hitting LastResort) must bind the
// default system typeface instead of dereferencing a null primary SkTypeface, and keep
// unresolvedGlyphs() and unresolvedCodepoints() synchronized across control/format characters
DEF_TEST(CoreTextPipeline_EmptyFontFamilies_LastResortAndUnresolvedSync, reporter) {
    auto fc = test_font_collection();
    ParagraphStyle ps;
    ps.setTextShapingEngine(TextShapingEngine::kCoreText);
    ps.setFakeMissingFontStyles(true);

    TextStyle ts;
    ts.setFontFamilies({});
    ts.setFontSize(16.0f);
    ts.setFontStyle(SkFontStyle::Bold());

    ParagraphBuilderImpl builder(ps, fc, get_unicode(ps));
    builder.pushStyle(ts);
    // U+10FFFF is a noncharacter guaranteed to fall through to LastResort, surrounded by ZWJ/ZWNJ
    // and newline controls that must not be recorded in unresolvedCodepoints
    builder.addText("A\n\xE2\x80\x8D\xF4\x8F\xBF\xBF\xE2\x80\x8B");
    builder.pop();

    auto p = builder.Build();
    p->layout(500.0f);

    REPORTER_ASSERT(reporter, p->unresolvedGlyphs() == 1, "unresolved=%d", p->unresolvedGlyphs());
    auto unresolved = p->unresolvedCodepoints();
    REPORTER_ASSERT(reporter, unresolved.size() == 1, "size=%zu", unresolved.size());
    REPORTER_ASSERT(reporter, unresolved.count(0x10FFFF) == 1);
    REPORTER_ASSERT(reporter, unresolved.count('\n') == 0);
    REPORTER_ASSERT(reporter, unresolved.count(0x200D) == 0);
    REPORTER_ASSERT(reporter, unresolved.count(0x200B) == 0);
}

// Multi-run ellipsis (e.g. Latin + Emoji or fallback characters) must coalesce into a single
// RunHandler buffer without triggering SkASSERT(!fRun) in TextLine::ShapeString
DEF_TEST(CoreTextPipeline_MultiRunEllipsisAndAdjacentStyleRuns, reporter) {
    if (!has_font_families({"Helvetica", "Apple Color Emoji"})) {
        return;
    }
    auto fc = test_font_collection();
    ParagraphStyle ps;
    ps.setTextShapingEngine(TextShapingEngine::kCoreText);
    ps.setMaxLines(1);
    ps.setEllipsis(SkString("...\xF0\x9F\x98\x80"));

    ParagraphBuilderImpl builder(ps, fc, get_unicode(ps));
    TextStyle regular;
    regular.setFontFamilies({SkString("Helvetica")});
    regular.setFontSize(18.0f);
    regular.setFontEdging(SkFont::Edging::kAlias);
    builder.pushStyle(regular);
    builder.addText("First span ");
    builder.pop();

    TextStyle subpixelBold = regular;
    subpixelBold.setFontEdging(SkFont::Edging::kSubpixelAntiAlias);
    builder.pushStyle(subpixelBold);
    builder.addText("Second span that is long enough to trigger ellipsis truncation");
    builder.pop();

    auto p = builder.Build();
    p->layout(140.0f);

    auto impl = static_cast<ParagraphImpl*>(p.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    REPORTER_ASSERT(reporter, p->didExceedMaxLines());
    REPORTER_ASSERT(reporter, impl->runs().size() >= 2);
    if (impl->runs().size() >= 2) {
        REPORTER_ASSERT(reporter, impl->runs()[0].font().getEdging() == SkFont::Edging::kAlias);
        REPORTER_ASSERT(reporter,
                        impl->runs()[1].font().getEdging() ==
                                SkFont::Edging::kSubpixelAntiAlias);
    }
}

// Complex scripts that reorder glyphs within a cluster (Devanagari pre-base matra U+093F in
// "ककि") must normalize CTRun cluster indices to monotonic text order so cluster table
// construction covers the full text range
DEF_TEST(CoreTextPipeline_NonMonotonicClusterIndices_DevanagariMatra, reporter) {
    auto fc = test_font_collection();
    ParagraphStyle ps;
    ps.setTextShapingEngine(TextShapingEngine::kCoreText);
    TextStyle ts;
    ts.setFontSize(20.0f);

    // "ककि" = U+0915 U+0915 U+093F (9 UTF-8 bytes)
    const char* deva = "\xE0\xA4\x95\xE0\xA4\x95\xE0\xA4\xBF";
    ParagraphBuilderImpl builder(ps, fc, get_unicode(ps));
    builder.pushStyle(ts);
    builder.addText(deva);
    builder.pop();

    auto p = builder.Build();
    p->layout(500.0f);

    auto impl = static_cast<ParagraphImpl*>(p.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    for (const auto& run : impl->runs()) {
        for (size_t k = 1; k <= run.size(); ++k) {
            if (run.leftToRight()) {
                REPORTER_ASSERT(reporter, run.clusterIndexes()[k] >= run.clusterIndexes()[k - 1]);
            }
        }
    }
    auto boxes = p->getRectsForRange(0, 3, RectHeightStyle::kTight, RectWidthStyle::kTight);
    REPORTER_ASSERT(reporter, !boxes.empty());
}

// CoreText places trailing whitespace in an RTL CTLine at negative X coordinates
// [-CTLineGetTrailingWhitespaceWidth, 0), so visualOriginX must start there to keep the last
// glyph's fPositions[k] consistent with the fPositions[glyphCount] advance sentinel
DEF_TEST(CoreTextPipeline_RtlTrailingWhitespaceClusterWidthAndUserOnlyFontCollection, reporter) {
    if (!has_font_families({"Helvetica", "Apple Color Emoji"})) {
        return;
    }
    auto fc = test_font_collection();
    ParagraphStyle ps;
    ps.setTextShapingEngine(TextShapingEngine::kCoreText);
    ps.setTextDirection(TextDirection::kRtl);

    // "אבג" vs "אבג    " (3 Hebrew letters + 4 trailing spaces)
    const char* rtlNoSpaces = "\xD7\x90\xD7\x91\xD7\x92";
    const char* rtlTrailingSpaces = "\xD7\x90\xD7\x91\xD7\x92    ";
    TextStyle ts;
    ts.setFontFamilies({SkString("Helvetica")});
    ts.setFontSize(14.0f);

    ParagraphBuilderImpl baseBuilder(ps, fc, get_unicode(ps));
    baseBuilder.pushStyle(ts);
    baseBuilder.addText(rtlNoSpaces);
    baseBuilder.pop();
    auto baseP = baseBuilder.Build();
    baseP->layout(500.0f);

    ParagraphBuilderImpl builder(ps, fc, get_unicode(ps));
    builder.pushStyle(ts);
    builder.addText(rtlTrailingSpaces);
    builder.pop();
    auto p = builder.Build();
    p->layout(500.0f);

    auto impl = static_cast<ParagraphImpl*>(p.get());
    REPORTER_ASSERT(reporter, impl->isCoreTextShaped());
    Paragraph::GlyphClusterInfo baseFirstCluster;
    Paragraph::GlyphClusterInfo firstCluster;
    REPORTER_ASSERT(reporter, baseP->getGlyphClusterAt(0, &baseFirstCluster));
    REPORTER_ASSERT(reporter, p->getGlyphClusterAt(0, &firstCluster));
    REPORTER_ASSERT(reporter,
                    SkScalarNearlyEqual(firstCluster.fBounds.width(),
                                        baseFirstCluster.fBounds.width(),
                                        0.01f),
                    "RTL first cluster width %f != no-trailing-spaces width %f",
                    firstCluster.fBounds.width(),
                    baseFirstCluster.fBounds.width());

    // When a client registers user fonts (AssetFontManager) without setting a defaultFontManager,
    // matchFamily must not resolve system families ahead of registered user families
    sk_sp<SkTypeface> emojiFace =
            ToolUtils::TestFontMgr()->matchFamilyStyle("Apple Color Emoji", SkFontStyle());
    REPORTER_ASSERT(reporter, emojiFace != nullptr);
    if (emojiFace) {
        auto assetProvider = sk_make_sp<TypefaceFontProvider>();
        assetProvider->registerTypeface(emojiFace, SkString("Apple Color Emoji"));
        auto userOnlyFc = sk_make_sp<FontCollection>();
        userOnlyFc->setAssetFontManager(assetProvider);

        auto matched = CoreTextFontRegistry::Instance().findTypefaces(
                *userOnlyFc,
                {SkString(".AppleSystemUIFont"), SkString("Apple Color Emoji")},
                SkFontStyle::Normal(),
                std::nullopt);
        REPORTER_ASSERT(reporter, matched.size() == 1, "matched=%zu", matched.size());
        if (matched.size() == 1) {
            SkString family;
            matched[0]->getFamilyName(&family);
            REPORTER_ASSERT(reporter,
                            family.equals("Apple Color Emoji"),
                            "expected Apple Color Emoji, got '%s'",
                            family.c_str());
        }
    }
}

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)
