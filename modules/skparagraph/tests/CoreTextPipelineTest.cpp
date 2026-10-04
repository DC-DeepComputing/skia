/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

#include "modules/skparagraph/src/CoreTextAttrStringBuilder.h"
#include "modules/skparagraph/src/CoreTextFontRegistry.h"
#include "modules/skparagraph/src/CoreTextIndexMap.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <string.h>

using namespace skia::textlayout;

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

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
